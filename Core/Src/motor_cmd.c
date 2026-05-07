#include "motor_cmd.h"
#include "stepperMotor.h"
#include "tim.h"
#include <stdio.h>
#include "motor_control.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
void v_motor_speed_update(uint16_t arr);

/*----------------------------------------------------------------------------*/
// little-endian helpers
static inline int32_t le_i32(const uint8_t* p)
{
  return (int32_t)((uint32_t)p[0] |
                   ((uint32_t)p[1] << 8) |
                   ((uint32_t)p[2] << 16) |
                   ((uint32_t)p[3] << 24));
}

static inline uint32_t le_u32(const uint8_t* p)
{
  return (uint32_t)p[0] |
         ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static inline int16_t le_i16(const uint8_t* p)
{
  return (int16_t)((uint16_t)p[0] |
                   ((uint16_t)p[1] << 8));
}

#define MOTOR_VEC_MAX_ITEMS  MOTOR_COUNT

static int16_t g_vec_delta[MOTOR_VEC_MAX_ITEMS];
static uint8_t g_vec_used[MOTOR_VEC_MAX_ITEMS];
static uint8_t g_vec_expected = 0;
static uint8_t g_vec_active = 0;
/*----------------------------------------------------------------------------*/
/*
پروتکل CAN (DLC=8):
ID = MOTOR_CMD_BASE_ID + board_id

CMD_STOP_ALL:
  data[0]=0x01

CMD_STOP_ONE:
  data[0]=0x02
  data[1]=motor_id (0..31)

CMD_MOVE_ONE:
  data[0]=0x03
  data[1]=motor_id (0..31)
  data[2..5]=steps_signed int32 (LE)
  (steps >0 => CW, steps <0 => CCW)

CMD_MOVE_MASK:
  data[0]=0x04
  data[1..4]=mask32 (LE)  (bit i => motor i)
  data[5]=dir (0=CW, 1=CCW)
  data[6..7]=steps_u16 (LE)   (0 ممنوع)

CMD_SET_SPEED:
  data[0]=0x05
  data[1..4]=speed_hz u32 (LE)
*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
void motor_cmd_dispatch(const CAN_Frame_t* f, uint8_t node_id)
{
  if (!f) return;
  if (f->dlc != 8) return;

  // Current node motor command ID:
  // ID = 0x300 + node_id
  //
  // For BOARD_ID = 1:
  // Motor command ID = 0x301
  if (f->id != (uint32_t)(CMD_BASE_ID + node_id)) return;

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
      // Emergency/software stop for all motors.
      Motor_StopAll();
      break;
    }

    case MOTOR_CMD_STOP_ONE:
    {
      // data[1] = motor index
      uint8_t idx = f->data[1];
      Motor_Stop(idx);
      break;
    }

    case MOTOR_CMD_JOG_ONE:
    {
      // Same as UART:
      // mj <idx> <delta>
      //
      // data[0] = 0x03
      // data[1] = motor index
      // data[2..3] = delta int16 little-endian
      uint8_t idx = f->data[1];
      int16_t delta = le_i16(&f->data[2]);

      MOTOR_CMD_LOG("JOG_ONE idx=%u delta=%d", idx, delta);

      Motor_JogSteps(idx, delta);
      break;
    }

    case MOTOR_CMD_JOG_MASK:
    {
      // Same as UART:
      // mjm <mask> <delta>
      //
      // data[0] = 0x04
      // data[1..4] = mask uint32 little-endian
      // data[5..6] = delta int16 little-endian
      uint32_t mask = le_u32(&f->data[1]);
      int16_t delta = le_i16(&f->data[5]);

      Motor_JogMaskSteps(mask, delta);
      break;
    }

    case MOTOR_CMD_HOME_ONE:
    {
      // Same as UART:
      // mhome <idx>
      //
      // data[1] = motor index
      //
      // IMPORTANT:
      // This is software home, not physical homing.
      uint8_t idx = f->data[1];
      Motor_GoHome(idx);
      break;
    }

    case MOTOR_CMD_HOME_ALL:
    {
      // Same as UART:
      // mhomeall
      //
      // Return all moved motors to software position zero.
      Motor_GoHomeAll();
      break;
    }

    case MOTOR_CMD_VEC_BEGIN:
    {
      // Same concept as UART:
      // mjv <idx0> <delta0> <idx1> <delta1> ...
      //
      // data[1] = expected item count
      //
      // Example:
      // if we want 3 motors in vector move:
      // data[1] = 3

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
      break;
    }

    case MOTOR_CMD_VEC_ITEM:
    {
      // Add up to two vector items per CAN frame.
      //
      // data[1] = motor_idx_0
      // data[2..3] = delta0 int16 LE
      // data[4] = motor_idx_1, or 0xFF if unused
      // data[5..6] = delta1 int16 LE
      //
      // data[7] reserved

      if (!g_vec_active)
          break;

      uint8_t idx0 = f->data[1];
      int16_t d0 = le_i16(&f->data[2]);

      if (idx0 < MOTOR_VEC_MAX_ITEMS)
      {
        g_vec_delta[idx0] = d0;
        g_vec_used[idx0] = 1;
      }

      uint8_t idx1 = f->data[4];
      int16_t d1 = le_i16(&f->data[5]);

      if (idx1 != 0xFF && idx1 < MOTOR_VEC_MAX_ITEMS)
      {
        g_vec_delta[idx1] = d1;
        g_vec_used[idx1] = 1;
      }

      break;
    }

    case MOTOR_CMD_VEC_COMMIT:
    {
      // Execute vector move.
      //
      // Current implementation:
      // Sequential execution, same as UART mjv.
      //
      // Later, if low-level supports real simultaneous vector motion,
      // only this backend needs to change.

      if (!g_vec_active)
          break;

      uint8_t executed = 0;

      for (uint8_t i = 0; i < MOTOR_VEC_MAX_ITEMS; i++)
      {
        if (g_vec_used[i])
        {
          Motor_JogSteps(i, g_vec_delta[i]);
          executed++;
        }
      }

      g_vec_active = 0;
      g_vec_expected = 0;

      (void)executed;
      break;
    }

    case MOTOR_CMD_SET_SPEED:
    {
      // Keep for later.
      // Current motor_control layer does not expose speed API yet.
      break;
    }

    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

