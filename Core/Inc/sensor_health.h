#ifndef INC_SENSOR_HEALTH_H_
#define INC_SENSOR_HEALTH_H_
/*----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/


//  Sensor Health = وضعیت سنسور + وضعیت تماس بدن + کیفیت سیگنال
// valid / present / loaded / noisy / no_update / suspect / fault
/*----------------------------------------------------------------------------*/


#define SH_SENSOR_COUNT 32


#define SH_SENSOR_COUNT        32

// اختلاف از baseline برای تشخیص فشار/بدن
#define SH_LOAD_THRESHOLD      4

// پنجره کوتاه برای بررسی نویز
#define SH_NOISE_WINDOW        8

// اگر بازه تغییرات در پنجره از این بیشتر شود، noisy است
#define SH_NOISE_THRESHOLD     8

// تعداد سیکل برای no-update تشخیصی
// NOTE:
// این fault نیست. فقط می‌گوید مقدار برای مدت طولانی ثابت بوده.
#define SH_NO_UPDATE_CYCLES    600   // حدود 60 ثانیه با loop صد میلی‌ثانیه
/*----------------------------------------------------------------------------*/

typedef struct
{
    uint8_t is_valid;       // سنسور داده قابل اعتماد دارد
    uint8_t is_loaded;      // مقدار نسبت به baseline تغییر کرده، یعنی احتمالاً بدن/فشار روی پیکسل است
    uint8_t is_noisy;       // نوسان غیرطبیعی در چند نمونه اخیر
    uint8_t is_no_update;   // مقدار برای مدت طولانی کاملاً ثابت مانده، فقط flag تشخیصی
    uint8_t is_stuck;       // stuck واقعی؛ فعلاً فقط توسط self-test یا actuator-test ست شود
    uint8_t fault;          // خطای قطعی سنسور
} sensor_health_t;
/*----------------------------------------------------------------------------*/

// مقداردهی اولیه health monitor
void SensorHealth_Init(void);

// پردازش health برای همه سنسورها
void SensorHealth_Process(void);

// گرفتن health یک سنسور
const sensor_health_t* SensorHealth_Get(uint8_t idx);

// گرفتن کل آرایه health
const sensor_health_t* SensorHealth_GetAll(void);
/*----------------------------------------------------------------------------*/


#endif /* INC_SENSOR_HEALTH_H_ */
