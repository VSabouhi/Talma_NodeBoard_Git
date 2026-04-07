#ifndef INC_STEPPERMOTOR_H_
#define INC_STEPPERMOTOR_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
typedef enum {
  STEPPER_OK = 0,
  STEPPER_BUSY,
  STEPPER_BAD_PARAM
} stepper_status_t;

typedef enum {
  STEPPER_DIR_CW  = 0,
  STEPPER_DIR_CCW = 1
} stepper_dir_t;
/*----------------------------------------------------------------------------*/
#define STEPPER_MOTOR_COUNT 32
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
// init GPIO mapping (از main.h استفاده می‌کند)
void stepper_init(void);
// سرعت مشترک (Hz): تعداد step در ثانیه
void stepper_set_speed_hz(uint32_t step_hz);
// حرکت مستقل با سرعت مشترک
stepper_status_t stepper_move_motor(uint8_t motor_id, int32_t steps_signed);
// حرکت گروهی (همه‌ی موتورها در mask)
// بیت i = 1 یعنی motor i شرکت دارد
stepper_status_t stepper_move_mask(uint32_t mask, int32_t steps_signed, stepper_dir_t dir);
/*----------------------------------------------------------------------------*/
void stepper_stop_motor(uint8_t motor_id);
void stepper_stop_all(void);
/*----------------------------------------------------------------------------*/
// وضعیت
uint32_t stepper_active_mask(void);
int32_t  stepper_remaining(uint8_t motor_id);
// این را از ISR تایمر صدا می‌زنی (هر Period = یک Step)
void stepper_on_tick_isr(void);
/*----------------------------------------------------------------------------*/


#endif /* INC_STEPPERMOTOR_H_ */
