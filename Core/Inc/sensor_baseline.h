#ifndef INC_SENSOR_BASELINE_H_
#define INC_SENSOR_BASELINE_H_
/*----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/

#define SB_SENSOR_COUNT       32
#define SB_SAMPLE_COUNT       64    // تعداد نمونه برای baseline
#define SB_VALID_MIN_MM       0     // حداقل مقدار معتبر
#define SB_VALID_MAX_MM       63    // حداکثر مقدار معتبر در payload

#define SB_SENSOR_COUNT       32
/*----------------------------------------------------------------------------*/

// وضعیت اجرای baseline
typedef enum
{
    SB_IDLE = 0,        // هنوز شروع نشده
    SB_RUNNING,         // در حال جمع آوری نمونه
    SB_DONE,            // با موفقیت تمام شده
    SB_FAILED           // شکست خورده
} sb_state_t;

// خروجی baseline برای هر سنسور
typedef struct
{
    uint16_t baseline[SB_SENSOR_COUNT];  // میانگین مقدار هر سنسور در حالت بدون فشار
    uint16_t noise[SB_SENSOR_COUNT];     // تخمین نویز اولیه (max - min)
    uint8_t  valid[SB_SENSOR_COUNT];     // آیا baseline این سنسور معتبر است؟
} sensor_baseline_data_t;
/*----------------------------------------------------------------------------*/

// شروع baseline گیری
void SensorBaseline_Start(void);

// پردازش baseline به صورت non-blocking
void SensorBaseline_Process(void);

// وضعیت baseline
sb_state_t SensorBaseline_GetState(void);

// آیا baseline تمام شده؟
bool SensorBaseline_IsDone(void);

// گرفتن خروجی baseline
const sensor_baseline_data_t* SensorBaseline_GetData(void);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

#endif /* INC_SENSOR_BASELINE_H_ */
