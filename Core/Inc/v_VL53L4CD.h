#ifndef INC_V_VL53L4CD_H_
#define INC_V_VL53L4CD_H_
/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------*/

/* اگر بخوای فرق “قطع شده” با “از اول نبوده” کاملاً واضح باشه، این قرارداد رو حفظ کن:

255 = Not present at boot (هیچوقت detect نشده)

251 = Was present, disconnected later

254 = Temporary I2C error (ممکنه لحظه‌ای)

0..50 = valid pressure distance
*/

/*----------------------------------------------------------*/

#define UART_RX_BUFFER_SIZE  40
#define SENSOR_COUNT         32

#define DEFAULT_I2C_ADDRESS  0x52

// ===== Data codes (what you see in ptrTof_1->data[]) =====
#define D_NO_SENSOR       255   // not detected at init / never existed
#define D_I2C_ERROR       254   // i2c error
#define D_STALLED         253   // no data ready for long time
#define D_RECOVERING      252   // in recovery action
#define D_DISCONNECTED    251   // was online, then disconnected during runtime
/*----------------------------------------------------------*/

typedef enum {
  S_OK            = 0,
  S_NO_SENSOR     = 1,
  S_I2C_ERROR     = 2,
  S_STALLED       = 3,
  S_RECOVERING    = 4,
  S_DISCONNECTED  = 5,
} sens_status_t;

struct tofStruct{
  int32_t data[SENSOR_COUNT];
};
/*----------------------------------------------------------*/

extern struct tofStruct *ptrTof_1;

// bus enable flags (true if at least one sensor detected on that bus)
extern bool v_I2C1_Bus, v_I2C2_Bus, v_I2C3_Bus, v_I2C4_Bus;

// online map per sensor (1=read it, 0=skip it)
extern uint8_t sens_online[SENSOR_COUNT];

// optional: status per sensor
extern sens_status_t sens_status[SENSOR_COUNT];
/*----------------------------------------------------------*/

// init helper
void initialize_ptrTof_1_data(void);

// init per bus
void VL53L4CD_Init_Multi_I2C1(void);
void VL53L4CD_Init_Multi_I2C2(void);
void VL53L4CD_Init_Multi_I2C3(void);
void VL53L4CD_Init_Multi_I2C4(void);

// read per bus
void VL53L4CD_SensorRead_I2C1(void);
void VL53L4CD_SensorRead_I2C2(void);
void VL53L4CD_SensorRead_I2C3(void);
void VL53L4CD_SensorRead_I2C4(void);

// اجرای offset calibration داخلی سنسور برای یک سنسور مشخص
// target_mm فاصله واقعی target از سنسور است.
// برای TALMA فعلاً در حالت بدون load مقدار 50mm استفاده می‌کنیم.
uint8_t VL53L4CD_CalibrateOffset_One(uint8_t idx, int16_t target_mm, int16_t *measured_offset_mm);
// وقتی 1 باشد، SensorTask نباید سنسورها را read کند.
// NOTE:
// برای calibration لازم است دسترسی به VL53L4CD انحصاری باشد.
extern volatile uint8_t g_sensor_cal_busy;
/*----------------------------------------------------------*/

#endif /* INC_V_VL53L4CD_H_ */
