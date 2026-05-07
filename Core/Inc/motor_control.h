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
#define MOTOR_MOVE_TIMEOUT_MARGIN_MS   500      // 500
#define MOTOR_MOVE_TIMEOUT_MAX_MS      10000       // 10000
/*----------------------------------------------------------------------------*/

typedef enum
{
    MOTOR_FAULT_NONE = 0,
    MOTOR_FAULT_COMMAND_TIMEOUT,
    MOTOR_FAULT_COMMAND_REJECTED
} motor_fault_t;
/*----------------------------------------------------------------------------*/

typedef struct
{
    int32_t current_pos;       // موقعیت تخمینی فعلی موتور بر حسب step
    int32_t target_pos;        // موقعیت هدف بر حسب step
    int32_t move_start_pos;    // موقعیت موتور در لحظه شروع حرکت
    int32_t commanded_steps;   // تعداد step فرمان داده‌شده در حرکت فعلی

    uint8_t is_moving;
    uint8_t fault;             // 1 یعنی موتور در fault نرم‌افزاری است
    motor_fault_t fault_type;   // نوع fault؛ stuck مکانیکی واقعی نیست مگر feedback داشته باشیم
} actuator_t;
/*----------------------------------------------------------------------------*/

// مقداردهی اولیه لایه موتور
void MotorControl_Init(void);

// تنظیم هدف برای یک موتور
void Motor_SetTarget(uint8_t idx, int16_t pos);

void Motor_GoHome(uint8_t idx);
void Motor_GoHomeAll(void);
void Motor_JogSteps(uint8_t idx, int16_t delta_steps);
// Jog multiple motors with the same delta.
// mask bit i = motor i.
// Example: mask 0x0000000F means motors 0,1,2,3.
void Motor_JogMaskSteps(uint32_t mask, int16_t delta_steps);
void Motor_ForceSetHome(uint8_t idx);

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
