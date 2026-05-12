#include "motor_control.h"
#include "stepperMotor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "motor_cmd.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/

// وضعیت همه actuatorها
static actuator_t g_motor[MOTOR_COUNT];

// زمان شروع حرکت هر موتور برای timeout
static TickType_t g_move_start[MOTOR_COUNT];
static TickType_t g_move_timeout[MOTOR_COUNT];
// محدود کردن target به بازه امن
static int16_t clamp_pos(int16_t pos)
{
    if (pos < MOTOR_POS_MIN) return MOTOR_POS_MIN;
    if (pos > MOTOR_POS_MAX) return MOTOR_POS_MAX;
    return pos;
}
/*----------------------------------------------------------------------------*/

static void update_estimated_position(uint8_t idx)
{
    int32_t rem = (int32_t)stepper_remaining(idx);
    int32_t done = g_motor[idx].commanded_steps - rem;

    // محافظت در برابر مقدارهای غیرمنتظره
    if (done < 0)
        done = 0;

    if (done > g_motor[idx].commanded_steps)
        done = g_motor[idx].commanded_steps;

    // اگر target بزرگ‌تر از start بوده، حرکت در جهت مثبت بوده
    // در غیر اینصورت در جهت منفی بوده.
    if (g_motor[idx].target_pos >= g_motor[idx].move_start_pos)
    {
        g_motor[idx].current_pos = g_motor[idx].move_start_pos + done;
    }
    else
    {
        g_motor[idx].current_pos = g_motor[idx].move_start_pos - done;
    }

    // position تخمینی را در محدوده مجاز نگه می‌داریم
    g_motor[idx].current_pos = clamp_pos(g_motor[idx].current_pos);
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
    	// در شروع سیستم، position فقط یک تخمین نرم‌افزاری است.
    	// چون سنسور home نداریم، صفر یعنی مبنای نرم‌افزاری فعلی.
    	g_motor[i].current_pos = 0;
    	g_motor[i].target_pos = 0;
    	g_motor[i].move_start_pos = 0;
    	g_motor[i].commanded_steps = 0;

    	g_motor[i].is_moving = 0;
    	g_motor[i].fault = 0;
    	g_motor[i].fault_type = MOTOR_FAULT_NONE;
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

    // محدود کردن position به بازه مجاز
    pos = clamp_pos(pos);

    int32_t current = g_motor[idx].current_pos;
    int32_t delta = pos - current;

    // اگر از قبل در همان position هستیم، حرکتی لازم نیست
    if (delta == 0)
    {
        g_motor[idx].target_pos = pos;
        g_motor[idx].is_moving = 0;
        return;
    }

    g_motor[idx].target_pos = pos;

    // حرکت یک موتور با استفاده از mask
    // NOTE:
    // از stepper_move_mask استفاده می‌کنیم چون در low-level
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

    // ثبت اطلاعات حرکت برای حسابداری نرم‌افزاری position
    // IMPORTANT:
    // چون سنسور موقعیت نداریم، تنها راه دانستن موقعیت موتور
    // همین شمارش stepهاست.
    // این اطلاعات در stop / timeout استفاده می‌شود.
    g_motor[idx].move_start_pos = current;
    g_motor[idx].commanded_steps = steps;
    // محاسبه timeout بر اساس تعداد step و سرعت فعلی موتور.
    // NOTE:
    // چون سرعت همه موتورها مشترک است، stepper_get_speed_hz مقدار global speed را می‌دهد.
    uint32_t hz = stepper_get_speed_hz();

    if (hz == 0)
    {
        // فرمان قابل اجرا نیست چون سرعت معتبر نداریم.
        // NOTE:
        // این stuck مکانیکی نیست؛ command rejected نرم‌افزاری است.
        g_motor[idx].fault = 1;
        g_motor[idx].fault_type = MOTOR_FAULT_COMMAND_REJECTED;
        g_motor[idx].is_moving = 0;
        return;
    }

    uint32_t expected_ms = (uint32_t)(((uint64_t)steps * 1000u) / hz);
    uint32_t timeout_ms = expected_ms + MOTOR_MOVE_TIMEOUT_MARGIN_MS;

    if (timeout_ms > MOTOR_MOVE_TIMEOUT_MAX_MS)
        timeout_ms = MOTOR_MOVE_TIMEOUT_MAX_MS;

    g_move_timeout[idx] = pdMS_TO_TICKS(timeout_ms);

    stepper_status_t st = stepper_move_mask(mask, steps, dir);

    if (st == STEPPER_OK)
    {
        // حرکت با موفقیت شروع شده
        g_motor[idx].is_moving = 1;

        // زمان شروع حرکت برای timeout
        g_move_start[idx] = xTaskGetTickCount();
    }
    else
    {
        // خطا در شروع فرمان حرکت.
        // NOTE:
        // این stuck مکانیکی را ثابت نمی‌کند؛ فقط یعنی low-level فرمان را قبول نکرد.
        g_motor[idx].fault = 1;
        g_motor[idx].fault_type = MOTOR_FAULT_COMMAND_REJECTED;
        g_motor[idx].is_moving = 0;
    }
}
/*----------------------------------------------------------------------------*/

void Motor_GoHome(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return;

    // Home نرم‌افزاری:
    // چون سنسور home نداریم، home یعنی position تخمینی صفر.
    // موتور فقط به اندازه current_pos ثبت‌شده برمی‌گردد، نه بیشتر.
    Motor_SetTarget(idx, MOTOR_POS_MIN);
}

/*----------------------------------------------------------------------------*/

void Motor_GoHomeAll(void)
{
    // mhomeall:
    // Return all moved motors back to software home (position = 0).
    //
    // IMPORTANT:
    // This is NOT physical homing.
    // Startup position is assumed to be home.
    // Each motor returns using its stored software position.

    // Safety: do not start home-all if any motor is currently moving.
    // This prevents corrupting software position bookkeeping.
    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if (g_motor[i].is_moving)
            return;
    }

    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        // Skip faulted motors.
        // Fault reset should be handled by a separate command later.
        if (g_motor[i].fault)
            continue;

        // Only motors that moved away from software home need to return.
        if (g_motor[i].current_pos != MOTOR_POS_MIN)
        {
            Motor_GoHome(i);
        }
    }
}

/*----------------------------------------------------------------------------*/

void Motor_JogSteps(uint8_t idx, int16_t delta_steps)
{
    if (idx >= MOTOR_COUNT)
        return;

    if (delta_steps == 0)
        return;

    // اگر موتور در حال حرکت است، فرمان jog جدید قبول نکن.
    // NOTE:
    // این کار جلوی خراب شدن حسابداری نرم‌افزاری position را می‌گیرد.
    if (g_motor[idx].is_moving)
        return;

    // اگر موتور fault دارد، jog قبول نکن.
    // NOTE:
    // بعداً می‌توانیم command جدا برای reset fault اضافه کنیم.
    if (g_motor[idx].fault)
        return;

    int32_t new_target = g_motor[idx].current_pos + delta_steps;

    // Jog هم از Motor_SetTarget عبور می‌کند
    // تا همه حسابداری‌ها، timeout و direction logic یکجا بماند.
    Motor_SetTarget(idx, (int16_t)new_target);
}


/*----------------------------------------------------------------------------*/

void Motor_JogMaskSteps(uint32_t mask, int16_t delta_steps)
{
    if (mask == 0)
        return;

    if (delta_steps == 0)
        return;

    int32_t steps = (delta_steps > 0) ? delta_steps : -delta_steps;
    stepper_dir_t dir = (delta_steps > 0) ? STEPPER_DIR_CW : STEPPER_DIR_CCW;

    // Check all selected motors before starting group move.
    // If one motor is invalid, moving, faulted, or out of range,
    // reject the whole group command.
    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if ((mask & (1u << i)) == 0)
            continue;

        if (g_motor[i].is_moving)
            return;

        if (g_motor[i].fault)
            return;

        int32_t new_target = g_motor[i].current_pos + delta_steps;

        if (new_target < MOTOR_POS_MIN || new_target > MOTOR_POS_MAX)
            return;
    }

    uint32_t hz = stepper_get_speed_hz();

    if (hz == 0)
        return;

    uint32_t expected_ms = (uint32_t)(((uint64_t)steps * 1000u) / hz);
    uint32_t timeout_ms = expected_ms + MOTOR_MOVE_TIMEOUT_MARGIN_MS;

    if (timeout_ms > MOTOR_MOVE_TIMEOUT_MAX_MS)
        timeout_ms = MOTOR_MOVE_TIMEOUT_MAX_MS;

    TickType_t now = xTaskGetTickCount();

    // Bookkeeping for all selected motors.
    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if ((mask & (1u << i)) == 0)
            continue;

        g_motor[i].move_start_pos = g_motor[i].current_pos;
        g_motor[i].target_pos = g_motor[i].current_pos + delta_steps;
        g_motor[i].commanded_steps = steps;
        g_motor[i].is_moving = 1;

        g_move_start[i] = now;
        g_move_timeout[i] = pdMS_TO_TICKS(timeout_ms);
    }

    stepper_status_t st = stepper_move_mask(mask, steps, dir);

    if (st != STEPPER_OK)
    {
        // Low-level rejected the command.
        // Mark all selected motors as command rejected.
        for (uint8_t i = 0; i < MOTOR_COUNT; i++)
        {
            if ((mask & (1u << i)) == 0)
                continue;

            g_motor[i].is_moving = 0;
            g_motor[i].fault = 1;
            g_motor[i].fault_type = MOTOR_FAULT_COMMAND_REJECTED;
        }
    }
}
/*----------------------------------------------------------------------------*/

void Motor_ForceSetHome(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return;

    // این تابع فقط برای recovery دستی است.
    // IMPORTANT:
    // این home واقعی سنسوردار نیست.
    // فقط به نرم‌افزار می‌گوییم موقعیت فعلی را صفر فرض کن.
    //
    // فقط وقتی استفاده شود که اپراتور مطمئن است موتور/لیداسکرو
    // به home مکانیکی برگشته است.
    Motor_Stop(idx);

    g_motor[idx].current_pos = MOTOR_POS_MIN;
    g_motor[idx].target_pos = MOTOR_POS_MIN;
    g_motor[idx].move_start_pos = MOTOR_POS_MIN;
    g_motor[idx].commanded_steps = 0;
    g_motor[idx].is_moving = 0;
    g_motor[idx].fault = 0;
    g_motor[idx].fault_type = MOTOR_FAULT_NONE;
}

/*----------------------------------------------------------------------------*/

void Motor_ResetFault(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return;

    // Do not reset fault while the motor is still moving.
    // NOTE:
    // Fault reset is a recovery action after motion has stopped.
    if (g_motor[idx].is_moving)
        return;

    // Clear only software fault state.
    // IMPORTANT:
    // Do not change current_pos here.
    // Without physical homing, current_pos is still the best software estimate.
    g_motor[idx].fault = 0;
    g_motor[idx].fault_type = MOTOR_FAULT_NONE;
    g_motor[idx].commanded_steps = 0;
}

/*----------------------------------------------------------------------------*/

void Motor_ResetFaultAll(void)
{
    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        // Reset only motors that are not moving.
        // NOTE:
        // Active motion must be stopped first using STOP command.
        if (!g_motor[i].is_moving)
        {
            Motor_ResetFault(i);
        }
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

        MOTOR_CTRL_LOG("PROC idx=%u moving=%u rem=%ld pos=%ld target=%ld",
                       i,
                       g_motor[i].is_moving,
                       (long)stepper_remaining(i),
                       (long)g_motor[i].current_pos,
                       (long)g_motor[i].target_pos);

        // اگر low-level می‌گوید step باقی مانده صفر است،
        // حرکت تمام شده و current_pos را برابر target می‌کنیم.
        if (stepper_remaining(i) == 0)
        {
            // حرکت کامل شده، پس position نرم‌افزاری به target رسیده است.
            g_motor[i].current_pos = g_motor[i].target_pos;
            g_motor[i].is_moving = 0;
            g_motor[i].commanded_steps = 0;
            MotorCmd_NotifyMotorDone(i);

            continue;
        }

        // timeout حرکت
        // NOTE:
        // اگر موتور بیش از زمان مجاز در حال حرکت بماند،
        // برای جلوگیری از خطر، متوقف و fault می‌شود.
        if ((now - g_move_start[i]) > g_move_timeout[i])
        {
            // قبل از توقف، position تخمینی را بر اساس stepهای انجام‌شده آپدیت می‌کنیم.
            // NOTE:
            // چون سنسور position نداریم، این بهترین تخمین نرم‌افزاری ماست.
            update_estimated_position(i);

            MOTOR_CTRL_LOG(
                "TIMEOUT idx=%u pos=%ld target=%ld rem=%ld elapsed=%lu",
                i,
                (long)g_motor[i].current_pos,
                (long)g_motor[i].target_pos,
                (long)stepper_remaining(i),
                (unsigned long)(now - g_move_start[i]));

            stepper_stop_motor(i);

            g_motor[i].is_moving = 0;
            g_motor[i].commanded_steps = 0;

            // Timeout یعنی دنباله پالس فرمان‌داده‌شده در بازه نرم‌افزاری مجاز کامل نشده.
            // IMPORTANT:
            // این stuck مکانیکی را ثابت نمی‌کند، چون feedback موتور نداریم.
            g_motor[i].fault = 1;
            g_motor[i].fault_type = MOTOR_FAULT_COMMAND_TIMEOUT;
            MotorCmd_NotifyMotorFault(i, MOTOR_STATUS_FAULT_TIMEOUT);
        }
    }
}
/*----------------------------------------------------------------------------*/

void Motor_Stop(uint8_t idx)
{
    if (idx >= MOTOR_COUNT)
        return;

    // اگر موتور در حال حرکت بوده، قبل از توقف position را تخمینی آپدیت کن.
    // IMPORTANT:
    // بدون این کار، هر stop وسط مسیر باعث می‌شود current_pos اشتباه بماند.
    if (g_motor[idx].is_moving)
    {
        update_estimated_position(idx);
    }

    stepper_stop_motor(idx);

    g_motor[idx].is_moving = 0;
    g_motor[idx].commanded_steps = 0;
}
/*----------------------------------------------------------------------------*/

void Motor_StopAll(void)
{
    // قبل از stop کلی، position همه موتورهایی که در حال حرکت هستند آپدیت می‌شود.
    // NOTE:
    // چون stop_all همه gateها را می‌بندد، بعد از آن دیگر نمی‌توانیم
    // از remaining برای تخمین position استفاده مفیدتری بگیریم.
    for (uint8_t i = 0; i < MOTOR_COUNT; i++)
    {
        if (g_motor[i].is_moving)
        {
            update_estimated_position(i);
            g_motor[i].is_moving = 0;
            g_motor[i].commanded_steps = 0;
        }
    }

    stepper_stop_all();
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
