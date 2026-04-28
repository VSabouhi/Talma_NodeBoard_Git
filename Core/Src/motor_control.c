#include "motor_control.h"
#include "stepperMotor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
/*----------------------------------------------------------------------------*/

// وضعیت همه actuatorها
static actuator_t g_motor[MOTOR_COUNT];

// زمان شروع حرکت هر موتور برای timeout
static TickType_t g_move_start[MOTOR_COUNT];

// محدود کردن target به بازه امن
static int16_t clamp_pos(int16_t pos)
{
    if (pos < MOTOR_POS_MIN) return MOTOR_POS_MIN;
    if (pos > MOTOR_POS_MAX) return MOTOR_POS_MAX;
    return pos;
}
/*----------------------------------------------------------------------------*/

void MotorControl_Init(void)
{
    // ریست کامل وضعیت نرم‌افزاری موتور
    // NOTE:
    // این تابع فقط state نرم‌افزاری را صفر می‌کند.
    // init سخت‌افزاری موتور همچنان در stepper_init انجام می‌شود.
    memset(g_motor, 0, sizeof(g_motor));
    memset(g_move_start, 0, sizeof(g_move_start));

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        g_motor[i].current_pos = 0;
        g_motor[i].target_pos = 0;
        g_motor[i].is_moving = 0;
        g_motor[i].fault = 0;
    }
}
/*----------------------------------------------------------------------------*/

void Motor_SetTarget(uint8_t idx, int16_t pos)
{
    if (idx >= MOTOR_COUNT)
        return;

    // اگر موتور fault دارد، فرمان جدید قبول نکن
    // NOTE:
    // بعداً reset fault را با command جدا اضافه می‌کنیم.
    if (g_motor[idx].fault)
        return;

    pos = clamp_pos(pos);

    int16_t current = g_motor[idx].current_pos;
    int16_t delta = pos - current;

    if (delta == 0)
    {
        g_motor[idx].target_pos = pos;
        g_motor[idx].is_moving = 0;
        return;
    }

    g_motor[idx].target_pos = pos;

    // حرکت یک موتور با استفاده از mask
    // NOTE:
    // از stepper_move_mask استفاده می‌کنیم چون در low-level شما
    // g_rem و active_mask را درست تنظیم می‌کند.
    uint32_t mask = (1u << idx);

    stepper_dir_t dir;
    int32_t steps;

    if (delta > 0)
    {
        dir = STEPPER_DIR_CW;
        steps = delta;
    }
    else
    {
        dir = STEPPER_DIR_CCW;
        steps = -delta;
    }

    stepper_status_t st = stepper_move_mask(mask, steps, dir);

    if (st == STEPPER_OK)
    {
        g_motor[idx].is_moving = 1;
        g_move_start[idx] = xTaskGetTickCount();
    }
    else
    {
        g_motor[idx].fault = 1;
        g_motor[idx].is_moving = 0;
    }
}
/*----------------------------------------------------------------------------*/

void Motor_Process(void)
{
    TickType_t now = xTaskGetTickCount();

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if (!g_motor[i].is_moving)
            continue;

        // اگر low-level می‌گوید step باقی مانده صفر است،
        // حرکت تمام شده و current_pos را برابر target می‌کنیم.
        if (stepper_remaining(i) == 0)
        {
            g_motor[i].current_pos = g_motor[i].target_pos;
            g_motor[i].is_moving = 0;
            continue;
        }

        // timeout حرکت
        // NOTE:
        // اگر موتور بیش از زمان مجاز در حال حرکت بماند،
        // برای جلوگیری از خطر، متوقف و fault می‌شود.
        if ((now - g_move_start[i]) > pdMS_TO_TICKS(MOTOR_MOVE_TIMEOUT_MS))
        {
            stepper_stop_motor(i);

            g_motor[i].is_moving = 0;
            g_motor[i].fault = 1;
        }
    }
}
/*----------------------------------------------------------------------------*/

void Motor_Stop(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return;

    // توقف نرم‌افزاری و سخت‌افزاری یک موتور
    stepper_stop_motor(idx);

    g_motor[idx].is_moving = 0;

    // NOTE:
    // current_pos را اینجا تغییر نمی‌دهیم چون دقیق نمی‌دانیم
    // موتور وسط مسیر کجا متوقف شده است.
}
/*----------------------------------------------------------------------------*/

void Motor_StopAll(void)
{
    stepper_stop_all();

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        g_motor[i].is_moving = 0;
    }
}
/*----------------------------------------------------------------------------*/

const actuator_t* Motor_Get(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return 0;

    return &g_motor[idx];
}
/*----------------------------------------------------------------------------*/

const actuator_t* Motor_GetAll(void)
{
    return g_motor;
}
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
