#include "sensor_baseline.h"
#include "v_VL53L4CD.h"
#include <string.h>
/*----------------------------------------------------------------------------*/

// -------------------------
// internal accumulators
// -------------------------

// جمع نمونه ها برای محاسبه میانگین
static uint32_t g_sum[SB_SENSOR_COUNT];

// کوچک ترین مقدار دیده شده برای تخمین نویز
static uint16_t g_min[SB_SENSOR_COUNT];

// بزرگ ترین مقدار دیده شده برای تخمین نویز
static uint16_t g_max[SB_SENSOR_COUNT];

// تعداد نمونه های جمع شده
static uint16_t g_sample_count = 0;

// state baseline
static sb_state_t g_state = SB_IDLE;

// خروجی نهایی baseline
static sensor_baseline_data_t g_data;

// -------------------------
// public API
// -------------------------
/*----------------------------------------------------------------------------*/

void SensorBaseline_Start(void)
{
    // ریست کامل ساختارهای داخلی برای شروع baseline جدید
    memset(g_sum, 0, sizeof(g_sum));
    memset(&g_data, 0, sizeof(g_data));

    for (int i = 0; i < SB_SENSOR_COUNT; i++)
    {
        g_min[i] = 0xFFFF;
        g_max[i] = 0;
    }

    g_sample_count = 0;
    g_state = SB_RUNNING;
}
/*----------------------------------------------------------------------------*/

void SensorBaseline_Process(void)
{
    if (g_state != SB_RUNNING)
        return;

    // از داده های پردازش شده فعلی سنسورها نمونه می گیریم
    for (int i = 0; i < SB_SENSOR_COUNT; i++)
    {
        int32_t v = ptrTof_1->data[i];

        // فقط داده های معتبر وارد baseline شوند
        // وضعیت های خطا مثل 251..255 نباید وارد baseline شوند
        if ((v >= SB_VALID_MIN_MM) && (v <= SB_VALID_MAX_MM) && (sens_status[i] == S_OK))
        {
            uint16_t u = (uint16_t)v;

            g_sum[i] += u;

            if (u < g_min[i]) g_min[i] = u;
            if (u > g_max[i]) g_max[i] = u;

            g_data.valid[i] = 1;
        }
    }

    g_sample_count++;

    // هنوز نمونه گیری کامل نشده
    if (g_sample_count < SB_SAMPLE_COUNT)
        return;

    // ساخت خروجی نهایی baseline
    for (int i = 0; i < SB_SENSOR_COUNT; i++)
    {
        if (g_data.valid[i])
        {
            g_data.baseline[i] = (uint16_t)(g_sum[i] / SB_SAMPLE_COUNT);

            // نویز ساده = بازه تغییرات در زمان baseline
            if (g_max[i] >= g_min[i])
                g_data.noise[i] = (uint16_t)(g_max[i] - g_min[i]);
            else
                g_data.noise[i] = 0;
        }
        else
        {
            g_data.baseline[i] = 0;
            g_data.noise[i] = 0;
        }
    }

    g_state = SB_DONE;
}
/*----------------------------------------------------------------------------*/

sb_state_t SensorBaseline_GetState(void)
{
    return g_state;
}
/*----------------------------------------------------------------------------*/

bool SensorBaseline_IsDone(void)
{
    return (g_state == SB_DONE);
}
/*----------------------------------------------------------------------------*/

const sensor_baseline_data_t* SensorBaseline_GetData(void)
{
    return &g_data;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
