#include "motor_cmd.h"
#include "motor_control.h"
#include "stepperMotor.h"
#include "utils.h"
#include "can.h"

#include <stdint.h>
#include <stdio.h>

/* ============================================================
 * Motor CAN Command Module
 * ============================================================
 *
 * Main -> Node:
 *   Motor command ID = MOTOR_CMD_BASE_ID + BOARD_ID
 *   Example BOARD_ID=1: 0x401
 *
 * Node -> Main:
 *   Motor status ID  = MOTOR_STATUS_BASE_ID + BOARD_ID
 *   Example BOARD_ID=1: 0x481
 *
 * This module converts CAN motor commands into motor_control API calls.
 *
 * IMPORTANT:
 * Do NOT call low-level stepper functions directly here.
 * Always use motor_control layer so software position, fault,
 * timeout and moving state remain valid.
 * ============================================================ */


/* ============================================================
 * Little-endian helpers
 * ============================================================
 *
 * CAN payload bytes are encoded little-endian.
 * Example:
 *   100 decimal = 0x0064 -> data = 64 00
 * ============================================================ */

static inline uint32_t le_u32(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static inline int16_t le_i16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] |
                     ((uint16_t)p[1] << 8));
}


/* ============================================================
 * Vector move buffer
 * ============================================================
 *
 * Used for CAN vector command:
 *
 *   0x08 = VEC_BEGIN
 *   0x09 = VEC_ITEM
 *   0x0A = VEC_COMMIT
 *
 * Current execution is sequential, same as UART mjv.
 * Protocol is ready for future simultaneous vector movement.
 * ============================================================ */

#define MOTOR_VEC_MAX_ITEMS  MOTOR_COUNT

static int16_t g_vec_delta[MOTOR_VEC_MAX_ITEMS];
static uint8_t g_vec_used[MOTOR_VEC_MAX_ITEMS];
static uint8_t g_vec_expected = 0;
static uint8_t g_vec_active = 0;


/* ============================================================
 * Motor feedback state
 * ============================================================
 *
 * ACK:
 *   Sent immediately after command is accepted.
 *
 * DONE:
 *   Sent when all motors related to the active motion command finish.
 *
 * FAULT:
 *   Sent when motor_control reports a motor fault.
 *
 * NOTE:
 * Current implementation tracks one active motion command at a time.
 * Main should not send a new motion command before DONE/FAULT unless
 * this is intentionally allowed later.
 * ============================================================ */

static uint8_t  g_motor_status_seq = 0;
static uint8_t  g_status_node_id = 1;

static uint8_t  g_last_motion_cmd = 0;
static uint32_t g_pending_done_mask = 0;


/* ============================================================
 * Local prototypes
 * ============================================================ */

static uint8_t MotorCmd_AnyBusy(void);
static uint8_t MotorCmd_AnyFault(void);

static void MotorCmd_SendStatus(uint8_t status_type,
                                uint8_t a,
                                uint8_t b,
                                uint8_t c);

static void MotorCmd_SendAck(uint8_t last_cmd, uint8_t result);
static void MotorCmd_SendDone(uint8_t last_cmd, uint8_t result);
static void MotorCmd_TrackMotion(uint8_t cmd, uint32_t motor_mask);
static void MotorCmd_ClearPendingMotionMask(uint32_t motor_mask);
static void MotorCmd_ClearAllPendingMotion(void);
/* ============================================================
 * Pending motion guard
 * ============================================================
 *
 * Returns 1 while one motion command is waiting for DONE/FAULT.
 *
 * NOTE:
 * Current feedback tracking supports one active motion command.
 * Main must wait for DONE/FAULT before sending another motion command.
 * If Main overlaps motion commands, Node rejects the new motion with BUSY.
 * ============================================================ */
static uint8_t MotorCmd_HasPendingMotion(void)
{
    return (g_pending_done_mask != 0u);
}
/* ============================================================
 * Motor summary helpers
 * ============================================================ */

static uint8_t MotorCmd_AnyBusy(void)
{
    const actuator_t *m = Motor_GetAll();

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if (m[i].is_moving)
            return 1;
    }

    return 0;
}

static uint8_t MotorCmd_AnyFault(void)
{
    const actuator_t *m = Motor_GetAll();

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if (m[i].fault)
            return 1;
    }

    return 0;
}


/* ============================================================
 * Send motor status frame
 * ============================================================
 *
 * CAN ID:
 *   MOTOR_STATUS_BASE_ID + BOARD_ID
 *
 * For BOARD_ID = 1:
 *   0x480 + 1 = 0x481
 *
 * ACK / DONE payload:
 *   data[0] = status_type
 *             0x80 = ACK
 *             0x81 = DONE
 *   data[1] = last_cmd
 *   data[2] = result
 *   data[3] = busy summary
 *   data[4] = fault summary
 *   data[5] = seq
 *   data[6] = 0
 *   data[7] = 0
 *
 * FAULT payload:
 *   data[0] = 0x82
 *   data[1] = active motion command
 *             Example:
 *               0x03 = JOG_ONE
 *               0x0A = VECTOR_COMMIT
 *
 *   data[2] = fault_code
 *   data[3] = motor_idx
 *   data[4] = busy summary
 *   data[5] = seq
 *   data[6] = 0
 *   data[7] = 0
 *
 * IMPORTANT:
 * FAULT is terminal completion for the active motion command.
 * DONE must NOT be emitted afterward for the same command.
 * ============================================================ */
static void MotorCmd_SendStatus(uint8_t status_type,
                                uint8_t a,
                                uint8_t b,
                                uint8_t c)
{
    (void)c;

    CAN_TxHeaderTypeDef txh = {0};
    uint32_t mailbox = 0;
    uint8_t data[8] = {0};

    txh.StdId = (uint32_t)(MOTOR_STATUS_BASE_ID + g_status_node_id);
    txh.IDE = CAN_ID_STD;
    txh.RTR = CAN_RTR_DATA;
    txh.DLC = 8;
    txh.TransmitGlobalTime = DISABLE;

    data[0] = status_type;

    if (status_type == MOTOR_STATUS_ACK ||
        status_type == MOTOR_STATUS_DONE)
    {
        data[1] = a;                    /* last_cmd */
        data[2] = b;                    /* result */
        data[3] = MotorCmd_AnyBusy();   /* busy summary */
        data[4] = MotorCmd_AnyFault();  /* fault summary */
        data[5] = g_motor_status_seq++; /* sequence */
    }
    else if (status_type == MOTOR_STATUS_FAULT)
    {
        data[1] = a;                    /* active motion cmd */
        data[2] = b;                    /* fault_code */
        data[3] = c;                    /* motor_idx */
        data[4] = MotorCmd_AnyBusy();   /* busy summary */
        data[5] = g_motor_status_seq++; /* sequence */
        data[6] = 0;
        data[7] = 0;
    }

    MOTOR_CMD_LOG("STATUS tx id=0x%03lX type=0x%02X data=%02X %02X %02X %02X %02X %02X %02X %02X",
                  txh.StdId,
                  status_type,
                  data[0], data[1], data[2], data[3],
                  data[4], data[5], data[6], data[7]);

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
        (void)HAL_CAN_AddTxMessage(&hcan1, &txh, data, &mailbox);
    }
    else
    {
        MOTOR_CMD_LOG("STATUS tx skipped: no free CAN mailbox");
    }
}
/* ============================================================*/
static void MotorCmd_SendAck(uint8_t last_cmd, uint8_t result)
{
    MotorCmd_SendStatus(MOTOR_STATUS_ACK, last_cmd, result, 0);
}

static void MotorCmd_SendDone(uint8_t last_cmd, uint8_t result)
{
    MotorCmd_SendStatus(MOTOR_STATUS_DONE, last_cmd, result, 0);
}


/* ============================================================
 * Send one motor position/status response
 * ============================================================
 *
 * Payload:
 *   data[0] = 0x86
 *   data[1] = motor_idx
 *   data[2] = current_pos L
 *   data[3] = current_pos H
 *   data[4] = target_pos L
 *   data[5] = target_pos H
 *   data[6] = moving
 *   data[7] = fault
 *
 * NOTE:
 * This is request-based feedback only.
 * Node does NOT periodically broadcast motor positions.
 * ============================================================ */
static void MotorCmd_SendPosResponse(uint8_t idx)
{
    const actuator_t *m = Motor_Get(idx);

    if (!m)
        return;

    CAN_TxHeaderTypeDef txh = {0};
    uint32_t mailbox = 0;
    uint8_t data[8] = {0};

    txh.StdId = (uint32_t)(MOTOR_STATUS_BASE_ID + g_status_node_id);
    txh.IDE = CAN_ID_STD;
    txh.RTR = CAN_RTR_DATA;
    txh.DLC = 8;
    txh.TransmitGlobalTime = DISABLE;

    data[0] = MOTOR_STATUS_POS_RESPONSE;
    data[1] = idx;
    data[2] = (uint8_t)(m->current_pos & 0xFF);
    data[3] = (uint8_t)((m->current_pos >> 8) & 0xFF);
    data[4] = (uint8_t)(m->target_pos & 0xFF);
    data[5] = (uint8_t)((m->target_pos >> 8) & 0xFF);
    data[6] = m->is_moving;
    data[7] = m->fault;

    MOTOR_CMD_LOG("POS_RESPONSE idx=%u pos=%d target=%d moving=%u fault=%u",
                  idx,
                  m->current_pos,
                  m->target_pos,
                  m->is_moving,
                  m->fault);

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
        (void)HAL_CAN_AddTxMessage(&hcan1, &txh, data, &mailbox);
    }
    else
    {
        MOTOR_CMD_LOG("POS_RESPONSE tx skipped: no free CAN mailbox");
    }
}



static void MotorCmd_TrackMotion(uint8_t cmd, uint32_t motor_mask)
{
    g_last_motion_cmd = cmd;
    g_pending_done_mask = motor_mask;
}

/* ============================================================
 * Clear tracked pending motion
 * ============================================================
 *
 * Used by STOP commands.
 *
 * NOTE:
 * STOP is a command-level termination.
 * It cancels pending DONE tracking for the stopped motor(s),
 * so the previous motion command will not emit DONE later.
 * ============================================================ */
static void MotorCmd_ClearPendingMotionMask(uint32_t motor_mask)
{
    g_pending_done_mask &= ~motor_mask;

    if (g_pending_done_mask == 0u)
    {
        g_last_motion_cmd = 0;
    }
}

/* ============================================================
 * Clear all tracked pending motion
 * ============================================================
 *
 * Used by STOP_ALL.
 *
 * NOTE:
 * After STOP_ALL, no previous motion command should remain active.
 * ============================================================ */
static void MotorCmd_ClearAllPendingMotion(void)
{
    g_pending_done_mask = 0u;
    g_last_motion_cmd = 0u;
}
/* ============================================================
 * Called by motor_control.c when a motor finishes movement
 * ============================================================ */

void MotorCmd_NotifyMotorDone(uint8_t motor_idx)
{
    if (motor_idx >= MOTOR_COUNT)
        return;

    if ((g_pending_done_mask & (1u << motor_idx)) == 0)
        return;

    g_pending_done_mask &= ~(1u << motor_idx);

}


/* ============================================================
 * Called by motor_control.c when a motor fault is detected
 * ============================================================ */

void MotorCmd_NotifyMotorFault(uint8_t motor_idx, uint8_t fault_code)
{
    if (motor_idx >= MOTOR_COUNT)
        return;

    g_pending_done_mask &= ~(1u << motor_idx);

    // If this was the last active motor in the tracked motion,
    // clear motion context completely.
    //
    // IMPORTANT:
    // FAULT is considered terminal completion for the motion.
    // Therefore DONE must NOT be emitted afterward.
    if (g_pending_done_mask == 0)
    {
        g_last_motion_cmd = 0;
    }

    // FAULT payload includes the active motion command
    // so Main can associate runtime failure with the correct motion.
    MotorCmd_SendStatus(MOTOR_STATUS_FAULT,
                        g_last_motion_cmd,
                        fault_code,
                        motor_idx);

    // FAULT is terminal completion for the active motion command.
    // Clear tracked motion context after reporting the FAULT.
    if (g_pending_done_mask == 0)
    {
        g_last_motion_cmd = 0;
    }
}


/* ============================================================
 * Main CAN motor command dispatcher
 * ============================================================
 *
 * Expected CAN ID:
 *   MOTOR_CMD_BASE_ID + node_id
 *
 * Example:
 *   node_id = 1 -> 0x401
 *
 * DLC must be 8.
 * ============================================================ */

void motor_cmd_dispatch(const CAN_Frame_t *f, uint8_t node_id)
{
    if (!f)
        return;

    if (f->dlc != 8)
        return;

    g_status_node_id = node_id;

    if (f->id != (uint32_t)(MOTOR_CMD_BASE_ID + node_id))
        return;

    const uint8_t cmd = f->data[0];

    MOTOR_CMD_LOG("DISPATCH id=0x%03lX cmd=0x%02X data=%02X %02X %02X %02X %02X %02X %02X %02X",
                  f->id,
                  cmd,
                  f->data[0], f->data[1], f->data[2], f->data[3],
                  f->data[4], f->data[5], f->data[6], f->data[7]);

    switch ((motor_cmd_id_t)cmd)
    {
        case MOTOR_CMD_STOP_ALL:
        {
            // Stop all motors immediately.
            // NOTE:
            // Motor_StopAll updates software position for moving motors.
            Motor_StopAll();

            // STOP_ALL terminates any tracked motion command.
            // IMPORTANT:
            // Do this after Motor_StopAll so position estimation can still use
            // low-level remaining steps before all motors are disabled.
            MotorCmd_ClearAllPendingMotion();

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_STOP_ONE:
        {
            uint8_t idx = f->data[1];

            if (idx >= MOTOR_COUNT)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            // Stop one motor immediately.
            // NOTE:
            // Motor_Stop updates software position if the motor was moving.
            Motor_Stop(idx);

            // STOP_ONE terminates DONE tracking for this motor only.
            // If this was the last pending motor, the previous command context
            // is cleared completely.
            MotorCmd_ClearPendingMotionMask(1u << idx);

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_JOG_ONE:
        {
            uint8_t idx = f->data[1];
            int16_t delta = le_i16(&f->data[2]);

            if (idx >= MOTOR_COUNT)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            // Reject overlapping motion commands.
            // NOTE:
            // Only one active motion command is tracked by g_pending_done_mask.
            if (MotorCmd_HasPendingMotion())
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_BUSY);
                break;
            }
            MOTOR_CMD_LOG("JOG_ONE idx=%u delta=%d", idx, delta);

            Motor_JogSteps(idx, delta);
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);

            if (delta != 0)
                MotorCmd_TrackMotion(cmd, (1u << idx));
            else
                MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);

            break;
        }

        case MOTOR_CMD_JOG_MASK:
        {
            uint32_t mask = le_u32(&f->data[1]);
            int16_t delta = le_i16(&f->data[5]);

            // Reject overlapping motion commands.
            // NOTE:
            // Main must wait for DONE/FAULT before sending another motion command.
            if (MotorCmd_HasPendingMotion())
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_BUSY);
                break;
            }

            MOTOR_CMD_LOG("JOG_MASK mask=0x%08lX delta=%d",
                          (unsigned long)mask,
                          delta);

            Motor_JogMaskSteps(mask, delta);
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);

            if (mask != 0 && delta != 0)
                MotorCmd_TrackMotion(cmd, mask);
            else
                MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);

            break;
        }

        case MOTOR_CMD_HOME_ONE:
        {
            uint8_t idx = f->data[1];

            if (idx >= MOTOR_COUNT)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            // Reject overlapping motion commands.
            // NOTE:
            // HOME_ONE is a motion command when current_pos is not home.
            if (MotorCmd_HasPendingMotion())
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_BUSY);
                break;
            }
            const actuator_t *m = Motor_Get(idx);
            uint32_t home_mask = 0;

            if (m && m->current_pos != MOTOR_POS_MIN)
                home_mask = (1u << idx);

            MOTOR_CMD_LOG("HOME_ONE idx=%u", idx);

            Motor_GoHome(idx);
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);

            if (home_mask != 0)
                MotorCmd_TrackMotion(cmd, home_mask);
            else
                MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);

            break;
        }

        case MOTOR_CMD_HOME_ALL:
        {
            uint32_t home_mask = 0;
            const actuator_t *m = Motor_GetAll();

            // Reject overlapping motion commands.
            // NOTE:
            // HOME_ALL may move multiple motors, so it must not overlap
            // with another active command tracked by g_pending_done_mask.
            if (MotorCmd_HasPendingMotion())
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_BUSY);
                break;
            }

            for (uint8_t i = 0; i < MOTOR_COUNT; i++)
            {
                if (m[i].current_pos != MOTOR_POS_MIN)
                    home_mask |= (1u << i);
            }

            MOTOR_CMD_LOG("HOME_ALL mask=0x%08lX", (unsigned long)home_mask);

            Motor_GoHomeAll();
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);

            if (home_mask != 0)
                MotorCmd_TrackMotion(cmd, home_mask);
            else
                MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);

            break;
        }

        case MOTOR_CMD_VEC_BEGIN:
        {
            uint8_t count = f->data[1];

            if (count > MOTOR_VEC_MAX_ITEMS)
                count = MOTOR_VEC_MAX_ITEMS;

            for (uint8_t i = 0; i < MOTOR_VEC_MAX_ITEMS; i++)
            {
                g_vec_delta[i] = 0;
                g_vec_used[i] = 0;
            }

            g_vec_expected = count;
            g_vec_active = 1;

            MOTOR_CMD_LOG("VEC_BEGIN count=%u", count);

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_VEC_ITEM:
        {
            if (!g_vec_active)
            {
                // VECTOR item without active VECTOR session.
                // NOTE:
                // This is a protocol/state validation error, not a runtime motor fault.
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_CMD);
                break;
            }

            uint8_t idx0 = f->data[1];
            int16_t d0 = le_i16(&f->data[2]);

            uint8_t idx1 = f->data[4];
            int16_t d1 = le_i16(&f->data[5]);

            if (idx0 >= MOTOR_VEC_MAX_ITEMS)
            {
                // Invalid first vector motor index.
                // NOTE:
                // Bad index is a command validation error.
                // Do not report it as runtime FAULT.
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            if (idx1 != 0xFF && idx1 >= MOTOR_VEC_MAX_ITEMS)
            {
                // Invalid second vector motor index.
                // NOTE:
                // 0xFF means "no second item"; any other out-of-range value is invalid.
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            g_vec_delta[idx0] = d0;
            g_vec_used[idx0] = 1;
            MOTOR_CMD_LOG("VEC_ITEM idx=%u delta=%d", idx0, d0);

            if (idx1 != 0xFF)
            {
                g_vec_delta[idx1] = d1;
                g_vec_used[idx1] = 1;
                MOTOR_CMD_LOG("VEC_ITEM idx=%u delta=%d", idx1, d1);
            }

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_VEC_COMMIT:
        {
            if (!g_vec_active)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_FAULT);
                break;
            }

            // Reject overlapping vector execution.
            // NOTE:
            // VEC_BEGIN/VEC_ITEM only build the buffer.
            // Actual motion starts at VEC_COMMIT.
            if (MotorCmd_HasPendingMotion())
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_BUSY);
                break;
            }
            uint32_t vec_mask = 0;
            uint8_t executed = 0;

            for (uint8_t i = 0; i < MOTOR_VEC_MAX_ITEMS; i++)
            {
                if (g_vec_used[i])
                {
                    Motor_JogSteps(i, g_vec_delta[i]);
                    vec_mask |= (1u << i);
                    executed++;
                }
            }

            g_vec_active = 0;
            g_vec_expected = 0;

            MOTOR_CMD_LOG("VEC_COMMIT executed=%u mask=0x%08lX",
                          executed,
                          (unsigned long)vec_mask);

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);

            if (vec_mask != 0)
                MotorCmd_TrackMotion(cmd, vec_mask);
            else
                MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);

            break;
        }

        case MOTOR_CMD_FAULT_RESET_ONE:
        {
            uint8_t idx = f->data[1];

            if (idx >= MOTOR_COUNT)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            // Clear software fault for one motor.
            // NOTE:
            // Node does not retry, home, or move the motor here.
            // Recovery decision remains on Main.
            Motor_ResetFault(idx);

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_FAULT_RESET_ALL:
        {
            // Clear software faults for all stopped motors.
            // NOTE:
            // This does not move motors and does not change software position.
            Motor_ResetFaultAll();

            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);
            break;
        }

        case MOTOR_CMD_SET_SPEED:
        {
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendDone(cmd, MOTOR_RESULT_OK);
            break;
        }
        case MOTOR_CMD_POS_QUERY:
        {
            uint8_t idx = f->data[1];

            if (idx >= MOTOR_COUNT)
            {
                MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_MOTOR);
                break;
            }

            // Request-based status:
            // Main asked for one motor state, so Node replies once.
            // No periodic motor status broadcast is used.
            MotorCmd_SendAck(cmd, MOTOR_RESULT_OK);
            MotorCmd_SendPosResponse(idx);
            break;
        }

        default:
        {
            MotorCmd_SendAck(cmd, MOTOR_RESULT_INVALID_CMD);
            break;
        }
    }
}
