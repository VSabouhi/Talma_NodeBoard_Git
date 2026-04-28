#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_
/*----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/

#define MOTOR_COUNT 32

// حداکثر position مجاز هر موتور بر حسب step
// NOTE:
// این مقدار فعلاً محافظ نرم‌افزاری است.
// بعداً باید بر اساس مکانیک واقعی تنظیم شود.
#define MOTOR_POS_MIN        0
#define MOTOR_POS_MAX        5000

// timeout حرکت موتور
// NOTE:
// اگر موتور بیش از این زمان در حال حرکت بماند، fault می‌گیرد.
#define MOTOR_MOVE_TIMEOUT_MS 10000

typedef struct
{
    int16_t current_pos;     // موقعیت تخمینی فعلی موتور بر حسب step
    int16_t target_pos;      // موقعیت هدف
    uint8_t is_moving;       // آیا موتور در حال حرکت است؟
    uint8_t fault;           // خطای موتور
} actuator_t;
/*----------------------------------------------------------------------------*/

// مقداردهی اولیه لایه موتور
void MotorControl_Init(void);

// تنظیم هدف برای یک موتور
void Motor_SetTarget(uint8_t idx, int16_t pos);

// پردازش وضعیت موتورها
void Motor_Process(void);

// توقف یک موتور
void Motor_Stop(uint8_t idx);

// توقف همه موتورها
void Motor_StopAll(void);

// گرفتن وضعیت یک موتور
const actuator_t* Motor_Get(uint8_t idx);

// گرفتن همه وضعیت‌ها
const actuator_t* Motor_GetAll(void);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/


#endif /* INC_MOTOR_CONTROL_H_ */
