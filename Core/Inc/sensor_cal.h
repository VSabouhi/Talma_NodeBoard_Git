#ifndef INC_SENSOR_CAL_H_
#define INC_SENSOR_CAL_H_

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_CAL_COUNT 32
#define SENSOR_CAL_SAMPLES 64   // تعداد نمونه برای baseline

// وضعیت state machine کالیبراسیون
typedef enum {
    CAL_IDLE = 0,     // هنوز شروع نشده
    CAL_RUNNING,      // در حال نمونه گیری
    CAL_DONE,         // با موفقیت تمام شده
    CAL_FAILED        // شکست خورده
} cal_state_t;

// داده های calibration برای هر سنسور
typedef struct {
    uint16_t baseline[SENSOR_CAL_COUNT];   // مقدار baseline هر سنسور
    uint16_t noise[SENSOR_CAL_COUNT];      // تخمین نویز هر سنسور
    uint8_t  valid[SENSOR_CAL_COUNT];      // آیا baseline این سنسور معتبر است؟
} sensor_cal_data_t;

// شروع کالیبراسیون
void SensorCal_Start(void);

// پردازش non-blocking کالیبراسیون
void SensorCal_Process(void);

// آیا کالیبراسیون تمام شده؟
bool SensorCal_IsDone(void);

// گرفتن state فعلی
cal_state_t SensorCal_GetState(void);

// گرفتن خروجی calibration
const sensor_cal_data_t* SensorCal_GetData(void);

#endif /* INC_SENSOR_CAL_H_ */
