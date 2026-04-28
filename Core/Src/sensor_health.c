#include "sensor_health.h"
#include "v_VL53L4CD.h"
#include "sensor_baseline.h"
#include <string.h>

static sensor_health_t g_health[SH_SENSOR_COUNT];

// آخرین مقدار معتبر برای بررسی no-update
static int32_t g_last_value[SH_SENSOR_COUNT];

// شمارنده ثابت ماندن مقدار
static uint16_t g_no_update_count[SH_SENSOR_COUNT];

// history کوتاه برای noise detection
static uint8_t g_hist[SH_SENSOR_COUNT][SH_NOISE_WINDOW];
static uint8_t g_hist_idx[SH_SENSOR_COUNT];
static uint8_t g_hist_filled[SH_SENSOR_COUNT];

static int32_t abs_diff_i32(int32_t a, int32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

void SensorHealth_Init(void)
{
    memset(g_health, 0, sizeof(g_health));
    memset(g_last_value, 0, sizeof(g_last_value));
    memset(g_no_update_count, 0, sizeof(g_no_update_count));
    memset(g_hist, 0, sizeof(g_hist));
    memset(g_hist_idx, 0, sizeof(g_hist_idx));
    memset(g_hist_filled, 0, sizeof(g_hist_filled));
}

static uint8_t SensorHealth_CalcNoise(uint8_t idx, uint8_t v)
{
    // مقدار جدید را داخل history قرار می‌دهیم
    // NOTE:
    // این پنجره کوتاه کمک می‌کند noise را از یک پرش لحظه‌ای بهتر تشخیص دهیم.
    g_hist[idx][g_hist_idx[idx]] = v;
    g_hist_idx[idx]++;

    if (g_hist_idx[idx] >= SH_NOISE_WINDOW)
    {
        g_hist_idx[idx] = 0;
        g_hist_filled[idx] = 1;
    }

    // تا وقتی پنجره پر نشده، noisy اعلام نکن
    if (!g_hist_filled[idx])
        return 0;

    uint8_t min_v = 255;
    uint8_t max_v = 0;

    for (uint8_t k = 0; k < SH_NOISE_WINDOW; k++)
    {
        uint8_t x = g_hist[idx][k];
        if (x < min_v) min_v = x;
        if (x > max_v) max_v = x;
    }

    // اگر بازه تغییرات در چند نمونه اخیر زیاد باشد، سنسور noisy است
    if ((max_v - min_v) > SH_NOISE_THRESHOLD)
        return 1;

    return 0;
}

void SensorHealth_Process(void)
{
    const sensor_baseline_data_t *base = SensorBaseline_GetData();

    for (uint8_t i = 0; i < SH_SENSOR_COUNT; i++)
    {
        int32_t v = ptrTof_1->data[i];

        // -------------------------
        // validity
        // -------------------------
        // NOTE:
        // اگر سنسور S_OK نباشد یا baseline معتبر نداشته باشد،
        // وارد تحلیل load/noise نمی‌شویم.
        if ((sens_status[i] != S_OK) || (!base->valid[i]) || (v < 0) || (v > 63))
        {
            g_health[i].is_valid = 0;
            g_health[i].is_loaded = 0;
            g_health[i].is_noisy = 0;
            g_health[i].is_no_update = 0;
            g_health[i].is_stuck = 0;

            // اینجا fault قطعی‌تر است چون داده معتبر نداریم
            g_health[i].fault = 1;

            g_no_update_count[i] = 0;
            g_last_value[i] = v;
            g_hist_filled[i] = 0;
            g_hist_idx[i] = 0;

            continue;
        }

        g_health[i].is_valid = 1;
        g_health[i].fault = 0;

        // -------------------------
        // load detection
        // -------------------------
        // NOTE:
        // اختلاف از baseline الزاماً خرابی نیست.
        // برای تخت یعنی احتمالاً بدن/فشار روی آن پیکسل وجود دارد.
        if (abs_diff_i32(v, (int32_t)base->baseline[i]) > SH_LOAD_THRESHOLD)
            g_health[i].is_loaded = 1;
        else
            g_health[i].is_loaded = 0;

        // -------------------------
        // noise detection
        // -------------------------
        // NOTE:
        // نویز را با پنجره کوتاه حساب می‌کنیم، نه فقط اختلاف دو نمونه.
        g_health[i].is_noisy = SensorHealth_CalcNoise(i, (uint8_t)v);

        // -------------------------
        // no-update detection
        // -------------------------
        // NOTE:
        // ثابت ماندن مقدار برای تخت طبیعی است.
        // بنابراین no_update فقط یک flag تشخیصی است، fault قطعی نیست.
        if (v == g_last_value[i])
        {
            if (g_no_update_count[i] < 0xFFFF)
                g_no_update_count[i]++;
        }
        else
        {
            g_no_update_count[i] = 0;
        }

        if (g_no_update_count[i] >= SH_NO_UPDATE_CYCLES)
            g_health[i].is_no_update = 1;
        else
            g_health[i].is_no_update = 0;

        // -------------------------
        // stuck
        // -------------------------
        // NOTE:
        // در runtime عادی stuck را از روی ثابت بودن مقدار تشخیص نمی‌دهیم.
        // stuck واقعی باید بعداً در self-test یا actuator-response ست شود.
        g_health[i].is_stuck = 0;

        g_last_value[i] = v;
    }
}

const sensor_health_t* SensorHealth_Get(uint8_t idx)
{
    if (idx >= SH_SENSOR_COUNT)
        return 0;

    return &g_health[idx];
}

const sensor_health_t* SensorHealth_GetAll(void)
{
    return g_health;
}
