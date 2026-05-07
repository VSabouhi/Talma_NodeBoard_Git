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
  MOTOR_CMD_VEC_COMMIT   = 0x0A
} motor_cmd_id_t;

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/

/**
 * فریم CAN را parse می‌کند و API موتور را صدا می‌زند.
 */
void motor_cmd_dispatch(const CAN_Frame_t* f, uint8_t node_id);

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


#endif /* INC_MOTOR_CMD_H_ */
