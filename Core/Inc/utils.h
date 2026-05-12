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
#define DBG_BUS_STATUS    0
#define DBG_MOTOR_TEST    0   // تست ساده موتور از DebugTask

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

/* ============================================================
 * TALMA Debug Configuration
 * ============================================================
 *
 * Global debug switches for firmware modules.
 *
 * Set to:
 *   1 -> enable logs
 *   0 -> disable logs
 *
 * These macros control runtime UART debug printing.
 *
 * IMPORTANT:
 * Keep all debug switches centralized here.
 * Do NOT spread local DEBUG defines across modules.
 * ============================================================ */

#define TALMA_DEBUG_CAN_RX        1
#define TALMA_DEBUG_MOTOR_CMD     0
#define TALMA_DEBUG_MOTOR_CTRL    0
#define TALMA_DEBUG_SENSOR        0

/* ============================================================
 * CAN RX Logging
 * ============================================================
 *
 * Logs raw received CAN frames.
 *
 * Used mainly in:
 *   freertos.c
 *   can_app.c
 *
 * Example output:
 *   [CAN_RX] id=0x301 dlc=8 ...
 * ============================================================ */

#if TALMA_DEBUG_CAN_RX
#define CAN_RX_LOG(fmt, ...) \
    printf("[CAN_RX] " fmt "\r\n", ##__VA_ARGS__)
#else
#define CAN_RX_LOG(fmt, ...) \
    do {} while (0)
#endif
/* ============================================================
 * Motor Command Logging
 * ============================================================
 *
 * Logs parsed motor commands received from CAN.
 *
 * Used mainly in:
 *   motor_cmd.c
 *
 * Example output:
 *   [MOTOR_CMD] JOG_ONE idx=0 delta=100
 * ============================================================ */
#if TALMA_DEBUG_MOTOR_CMD
#define MOTOR_CMD_LOG(fmt, ...) \
    printf("[MOTOR_CMD] " fmt "\r\n", ##__VA_ARGS__)
#else
#define MOTOR_CMD_LOG(fmt, ...) \
    do {} while (0)
#endif
/* ============================================================
 * Motor Control Logging
 * ============================================================
 *
 * Logs high-level motor control state changes.
 *
 * Intended for:
 *   motor_control.c
 *
 * Examples:
 *   timeout
 *   fault
 *   home
 *   target updates
 * ============================================================ */
#if TALMA_DEBUG_MOTOR_CTRL
#define MOTOR_CTRL_LOG(fmt, ...) \
    printf("[MOTOR_CTRL] " fmt "\r\n", ##__VA_ARGS__)
#else
#define MOTOR_CTRL_LOG(fmt, ...) \
    do {} while (0)
#endif

/* ============================================================
 * Sensor Logging
 * ============================================================
 *
 * Logs sensor-related activity.
 *
 * Intended for:
 *   v_VL53L4CD.c
 *   v_Sensor.c
 *   sensor_cal.c
 *
 * Examples:
 *   calibration
 *   reconnect
 *   timeout
 *   filtering
 * ============================================================ */
#if TALMA_DEBUG_SENSOR
#define SENSOR_LOG(fmt, ...) \
    printf("[SENSOR] " fmt "\r\n", ##__VA_ARGS__)
#else
#define SENSOR_LOG(fmt, ...) \
    do {} while (0)
#endif

#endif /* INC_UTILS_H_ */
