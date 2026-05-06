#ifndef INC_NODE_TEST_H_
#define INC_NODE_TEST_H_

#include <stdint.h>

// ================= TEST SWITCHES =================
// NOTE:
// هر تست را از اینجا فعال/غیرفعال کن.
// 1 = فعال
// 0 = غیرفعال

#define TEST_ENABLE_MOTOR_ONE       0  // تست فقط یک پیکسل
#define TEST_MOTOR_INDEX            1   // پیکسل 1 یعنی index=0 چون index از صفر شروع می‌شودست مستقیم موتور پیکسل 1
#define TEST_ENABLE_SENSOR_PRINT    0   // رزرو برای تست سنسور
#define TEST_ENABLE_HEALTH_PRINT    0   // رزرو برای تست health
#define TEST_ENABLE_CAN_TEST        0   // رزرو برای تست CAN

#define TEST_ENABLE_SENSOR_OFFSET_CAL   0
#define TEST_SENSOR_INDEX               0
#define TEST_SENSOR_OFFSET_TARGET_MM    50

#define TEST_ENABLE_MANUAL_JOG      0
#define TEST_JOG_MOTOR_INDEX        1
#define TEST_JOG_STEPS              50
#define TEST_JOG_DIRECTION          1   // 1=positive, 0=negative


#define TEST_ENABLE_UART_CLI      1

#define TEST_MOTOR_MOVE_MS        1000
#define TEST_MOTOR_PAUSE_MS       1000
// Task اصلی تست‌ها
void TestTask(void *argument);

#endif /* INC_NODE_TEST_H_ */
