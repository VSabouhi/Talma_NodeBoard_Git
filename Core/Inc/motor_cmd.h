/*
 * motor_cmd.h
 *
 *  Created on: Mar 2, 2026
 *      Author: Vahid.Sabouhi
 */

#ifndef INC_MOTOR_CMD_H_
#define INC_MOTOR_CMD_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "can_app.h"
/*----------------------------------------------------------------------------*/
#define CMD_BASE_ID  0x300
/* Motor-specific command CAN ID base.
 *
 * General node commands stay on:
 *   0x300 + BOARD_ID
 *
 * Motor commands use:
 *   0x400 + BOARD_ID
 *
 * Example:
 *   BOARD_ID = 1
 *   General command ID = 0x301
 *   Motor command ID   = 0x401
 */
#define MOTOR_CMD_BASE_ID  0x400
/* Motor feedback/status CAN base ID.
 *
 * Node reports motor command result on:
 *   0x480 + BOARD_ID
 *
 * Example:
 *   BOARD_ID = 1 -> 0x481
 */
#define MOTOR_STATUS_BASE_ID  0x480
/*----------------------------------------------------------------------------*/
typedef enum {
  MOTOR_CMD_STOP_ALL     = 0x01,
  MOTOR_CMD_STOP_ONE     = 0x02,

  // Same as UART: mj <idx> <delta>
  MOTOR_CMD_JOG_ONE      = 0x03,

  // Same as UART: mjm <mask> <delta>
  MOTOR_CMD_JOG_MASK     = 0x04,

  MOTOR_CMD_SET_SPEED    = 0x05,

  // Same as UART: mhome <idx>
  MOTOR_CMD_HOME_ONE     = 0x06,

  // Same as UART: mhomeall
  MOTOR_CMD_HOME_ALL     = 0x07,

  // Same as UART: mjv <idx0> <delta0> ...
  MOTOR_CMD_VEC_BEGIN    = 0x08,
  MOTOR_CMD_VEC_ITEM     = 0x09,
  MOTOR_CMD_VEC_COMMIT   = 0x0A,

  // Clear software fault for one motor.
  // NOTE:
  // This does NOT move the motor and does NOT perform physical homing.
  // It only allows Main to recover a motor after inspecting the fault.
  MOTOR_CMD_FAULT_RESET_ONE = 0x0B,

  // Clear software fault for all motors.
  // NOTE:
  // Main remains responsible for deciding whether retry/home is safe.
  MOTOR_CMD_FAULT_RESET_ALL = 0x0C,
  // Query one motor software position/status.
  // NOTE:
  // Node does not periodically broadcast motor position.
  // Position is reported only when Main explicitly asks.
  MOTOR_CMD_POS_QUERY    = 0x85
} motor_cmd_id_t;


typedef enum {
  MOTOR_STATUS_ACK    = 0x80,
  MOTOR_STATUS_DONE   = 0x81,
  MOTOR_STATUS_FAULT  = 0x82,
  MOTOR_STATUS_POS_RESPONSE = 0x86
} motor_status_msg_t;

typedef enum {
  MOTOR_RESULT_OK            = 0,
  MOTOR_RESULT_INVALID_CMD   = 1,
  MOTOR_RESULT_INVALID_MOTOR = 2,
  MOTOR_RESULT_BUSY          = 3,
  MOTOR_RESULT_LIMIT         = 4,
  MOTOR_RESULT_QUEUE_FULL    = 5,
  MOTOR_RESULT_FAULT         = 6
} motor_result_t;

typedef enum {
  MOTOR_STATUS_FAULT_STALL         = 1,
  MOTOR_STATUS_FAULT_LIMIT_HIT     = 2,
  MOTOR_STATUS_FAULT_DRIVER        = 3,
  MOTOR_STATUS_FAULT_TIMEOUT       = 4,
  MOTOR_STATUS_FAULT_INVALID_STATE = 5
} motor_status_fault_t;
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
void MotorCmd_NotifyMotorDone(uint8_t motor_idx);
void MotorCmd_NotifyMotorFault(uint8_t motor_idx, uint8_t fault_code);
/**
 * فریم CAN را parse می‌کند و API موتور را صدا می‌زند.
 */
void motor_cmd_dispatch(const CAN_Frame_t* f, uint8_t node_id);

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


#endif /* INC_MOTOR_CMD_H_ */
