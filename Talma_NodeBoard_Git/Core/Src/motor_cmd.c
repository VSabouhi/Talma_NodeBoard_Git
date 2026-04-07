#include "motor_cmd.h"
#include "stepperMotor.h"
#include "tim.h"
#include <stdio.h>

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
  if (f->id != (uint32_t)(CMD_BASE_ID + node_id)) return;

  const uint8_t cmd = f->data[0];
  const uint8_t a0  = f->data[1];
  const uint8_t a1  = f->data[2];
  const uint8_t a2  = f->data[3];
  const uint32_t param32 = le_u32(&f->data[4]);

  // ✅ این پرینت خوبه (اختیاری)
  //printf("DISPATCH OK: cmd=0x%02X a0=%u a1=%u a2=%u p32=%lu\r\n", cmd, a0, a1, a2, (unsigned long)param32);


  switch ((motor_cmd_id_t)cmd)
  {
    case MOTOR_CMD_STOP_ALL:
      stepper_stop_all();
      break;

    case MOTOR_CMD_STOP_ONE:
      stepper_stop_motor(a0);
      break;

    case MOTOR_CMD_MOVE_ONE:
    {

      int32_t steps = (int32_t)param32;
      if (a1) steps = -steps;     // a1=dir (0 CW / 1 CCW)
      //printf("CALL stepper_move_motor(motor=%u, steps=%ld)\r\n", a0, (long)steps);
      (void)stepper_move_motor(a0, steps);
      break;
    }

    case MOTOR_CMD_MOVE_MASK:
    {
      uint32_t mask = param32;
      uint16_t steps_u16 = (uint16_t)((uint16_t)a0 | ((uint16_t)a1 << 8));
      stepper_dir_t dir = (a2 ? STEPPER_DIR_CCW : STEPPER_DIR_CW);

      if (steps_u16 != 0)
        (void)stepper_move_mask(mask, (int32_t)steps_u16, dir);

      break;
    }

    case MOTOR_CMD_SET_SPEED:
      break;

    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

