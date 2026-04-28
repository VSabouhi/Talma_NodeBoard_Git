#ifndef INC_UTILS_H_
#define INC_UTILS_H_
/*----------------------------------------------------------*/

#include <stdint.h>

/*----------------------------------------------------------*/

static inline uint8_t clamp_u8(int32_t v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}


// ================= DEBUG SWITCHES =================
// NOTE:
// 1 = فعال ، 0 = غیرفعال

#define DBG_HEALTH        0
#define DBG_SENSOR_DATA   0
#define DBG_BUS_STATUS    1


// ================= DEBUG MACROS =================

// چاپ health سنسورها
#if DBG_HEALTH
#define DBG_PRINT_HEALTH(...)   printf(__VA_ARGS__)
#else
#define DBG_PRINT_HEALTH(...)
#endif

// چاپ دیتا سنسورها
#if DBG_SENSOR_DATA
#define DBG_PRINT_SENSOR(...)   printf(__VA_ARGS__)
#else
#define DBG_PRINT_SENSOR(...)
#endif

// چاپ وضعیت BUS
#if DBG_BUS_STATUS
#define DBG_PRINT_BUS(...)   printf(__VA_ARGS__)
#else
#define DBG_PRINT_BUS(...)
#endif
/*----------------------------------------------------------*/

#endif /* INC_UTILS_H_ */
