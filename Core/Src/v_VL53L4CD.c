#include "v_VL53L4CD.h"

#include "FreeRTOS.h"
#include "task.h"

#include "vl53l4cd_api.h"
#include "platform.h"

#include "main.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// ================== Types ==================
typedef enum { BUS_I2C1=0, BUS_I2C2=1, BUS_I2C3=2, BUS_I2C4=3 } bus_id_t;

// ================== Globals ==================
bool v_I2C1_Bus = false;
bool v_I2C2_Bus = false;
bool v_I2C3_Bus = false;
bool v_I2C4_Bus = false;

uint8_t sens_online[SENSOR_COUNT] = {0};
sens_status_t sens_status[SENSOR_COUNT] = {0};

// was online at least once?
static uint8_t g_everOnline[SENSOR_COUNT] = {0};

// per-sensor error streak + last good
static uint8_t  g_errStreak[SENSOR_COUNT]  = {0};
static uint32_t g_lastGoodMs[SENSOR_COUNT] = {0};

// per-bus throttle for bus recover
static uint32_t g_lastBusRecoverMs[4] = {0};

// results
static VL53L4CD_ResultsData_t results[SENSOR_COUNT];

// ptrTof instance
static struct tofStruct tofInstance = {0};
struct tofStruct *ptrTof_1 = &tofInstance;

// retry timer per sensor (when offline due to disconnect)
static uint32_t g_nextRetryMs[SENSOR_COUNT] = {0};
#define OFFLINE_RETRY_MS   5000U   // every 5 seconds

// Spike Removal

static int32_t g_lastFiltered[SENSOR_COUNT] = {0};
// مشخص می‌کند فیلتر هر سنسور حداقل یک نمونه معتبر گرفته یا نه
// NOTE: برای جلوگیری از قفل شدن فیلتر روی مقدار اولیه صفر
static uint8_t g_filterInitialized[SENSOR_COUNT] = {0};
// ================== Your XSHUT mapping ==================
GPIO_TypeDef* xshut_ports[SENSOR_COUNT] =
{ GPIOD, GPIOD, GPIOJ, GPIOG,
  GPIOK, GPIOK, GPIOE, GPIOE,
  GPIOI, GPIOI, GPIOI, GPIOF,
  GPIOF, GPIOC, GPIOH, GPIOA,
  GPIOB, GPIOJ, GPIOF, GPIOE,
  GPIOE, GPIOB, GPIOH, GPIOB,
  GPIOD, GPIOD, GPIOJ, GPIOK,
  GPIOG, GPIOG, GPIOC, GPIOA,
  GPIOH
};

uint16_t xshut_pins[SENSOR_COUNT] =
{
  GPIO_PIN_0, GPIO_PIN_4, GPIO_PIN_14, GPIO_PIN_11,
  GPIO_PIN_3, GPIO_PIN_7, GPIO_PIN_1, GPIO_PIN_3,
  GPIO_PIN_8, GPIO_PIN_11, GPIO_PIN_14, GPIO_PIN_6,
  GPIO_PIN_10, GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_3,
  GPIO_PIN_2, GPIO_PIN_2, GPIO_PIN_12, GPIO_PIN_7,
  GPIO_PIN_12, GPIO_PIN_10, GPIO_PIN_9, GPIO_PIN_12,
  GPIO_PIN_10, GPIO_PIN_15, GPIO_PIN_9, GPIO_PIN_1,
  GPIO_PIN_4, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_11,
  GPIO_PIN_15
};

// ================== Sensor I2C addresses ==================
static uint8_t dev[SENSOR_COUNT] = {
  0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E, 0x60, 0x62,
  0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
  0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
  0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E
};

// ================== Helpers ==================

static void I2C_Scan7bit(I2C_HandleTypeDef *hi2c, const char *tag)
{
  printf("%s scan start...\r\n", tag);
  for (uint8_t a = 1; a < 0x7F; a++)
  {
    // HAL expects 8-bit address (7bit<<1)
    if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(a << 1), 1, 10) == HAL_OK)
    {
      printf("%s FOUND: 7bit=0x%02X 8bit=0x%02X\r\n", tag, a, (unsigned)(a << 1));
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  printf("%s scan end.\r\n", tag);
}
/*----------------------------------------------------------*/

void initialize_ptrTof_1_data(void)
{
  for (int i=0;i<SENSOR_COUNT;i++) {
    ptrTof_1->data[i] = 50;
    sens_online[i] = 0;
    sens_status[i] = S_NO_SENSOR;
    g_everOnline[i] = 0;
    g_errStreak[i]  = 0;
    g_lastGoodMs[i] = 0;
    g_nextRetryMs[i] = 0;


    g_lastFiltered[i] = 0;

    // NOTE:
    // بعد از reset، اولین مقدار معتبر باید مستقیم وارد فیلتر شود
    g_filterInitialized[i] = 0;
  }
}
/*----------------------------------------------------------*/
static inline void ScheduleRetry(uint8_t i, uint32_t now)
{
  g_nextRetryMs[i] = now + OFFLINE_RETRY_MS;
}
/*----------------------------------------------------------*/

static void SetBusHandle(bus_id_t bus)
{
  switch(bus) {
    case BUS_I2C1: VL53L4CD_SetI2CHandle(&hi2c1); break;
    case BUS_I2C2: VL53L4CD_SetI2CHandle(&hi2c2); break;
    case BUS_I2C3: VL53L4CD_SetI2CHandle(&hi2c3); break;
    case BUS_I2C4: VL53L4CD_SetI2CHandle(&hi2c4); break;
  }
}
/*----------------------------------------------------------*/

static I2C_HandleTypeDef* GetBusHi2c(bus_id_t bus)
{
  switch(bus) {
    case BUS_I2C1: return &hi2c1;
    case BUS_I2C2: return &hi2c2;
    case BUS_I2C3: return &hi2c3;
    case BUS_I2C4: return &hi2c4;
    default: return &hi2c1;
  }
}
/*----------------------------------------------------------*/

static void RecoverBus(bus_id_t bus)
{
  // throttle happens in ReadBus_Generic
  switch(bus)
  {
    case BUS_I2C1:
      HAL_I2C_DeInit(&hi2c1);
      vTaskDelay(pdMS_TO_TICKS(10));
      MX_I2C1_Init();
      vTaskDelay(pdMS_TO_TICKS(10));
      VL53L4CD_SetI2CHandle(&hi2c1);
      break;

    case BUS_I2C2:
      HAL_I2C_DeInit(&hi2c2);
      vTaskDelay(pdMS_TO_TICKS(10));
      MX_I2C2_Init();
      vTaskDelay(pdMS_TO_TICKS(10));
      VL53L4CD_SetI2CHandle(&hi2c2);
      break;

    case BUS_I2C3:
      HAL_I2C_DeInit(&hi2c3);
      vTaskDelay(pdMS_TO_TICKS(10));
      MX_I2C3_Init();
      vTaskDelay(pdMS_TO_TICKS(10));
      VL53L4CD_SetI2CHandle(&hi2c3);
      break;

    case BUS_I2C4:
      HAL_I2C_DeInit(&hi2c4);
      vTaskDelay(pdMS_TO_TICKS(10));
      MX_I2C4_Init();
      vTaskDelay(pdMS_TO_TICKS(10));
      VL53L4CD_SetI2CHandle(&hi2c4);
      break;
  }
}
/*----------------------------------------------------------*/

// power-cycle one sensor (XSHUT) and re-init to its dev[i]
static int ResetAndReinitOne(bus_id_t bus, uint8_t i)
{
  uint16_t sensor_id = 0;
  int st;

  // power-cycle
  HAL_GPIO_WritePin(xshut_ports[i], xshut_pins[i], GPIO_PIN_RESET);
  vTaskDelay(pdMS_TO_TICKS(20));
  HAL_GPIO_WritePin(xshut_ports[i], xshut_pins[i], GPIO_PIN_SET);
  vTaskDelay(pdMS_TO_TICKS(50));

  SetBusHandle(bus);

  st = VL53L4CD_GetSensorId(DEFAULT_I2C_ADDRESS, &sensor_id);
  if (st != 0 || sensor_id != 0xEBAA) return -1;

  st = VL53L4CD_SensorInit(DEFAULT_I2C_ADDRESS);
  if (st != 0) return -2;

  st = VL53L4CD_SetI2CAddress(DEFAULT_I2C_ADDRESS, dev[i]);
  if (st != 0) return -3;

  st = VL53L4CD_StartRanging(dev[i]);
  if (st != 0) return -4;

  // NOTE:
  // بعد از reinit سنسور، فیلتر باید دوباره از اولین نمونه معتبر شروع کند
  g_lastFiltered[i] = 0;
  g_filterInitialized[i] = 0;

  return 0;
}
/*----------------------------------------------------------*/


//===================================================
// ================== Generic INIT ==================
//===================================================

static void InitBus_Generic(bus_id_t bus, uint8_t start, uint8_t end)
{
  uint16_t sensor_id = 0;
  int st = 0;
  uint8_t ok = 0;

  SetBusHandle(bus);

 // printf("I2C%u INIT: start=%u end=%u\r\n", (unsigned)(bus+1), (unsigned)start, (unsigned)end);

  if (bus == BUS_I2C1)
  {
    I2C_Scan7bit(&hi2c1, "I2C1");
  }

  // 1) power-off all sensors on this bus
  for (uint8_t i = start; i < end; i++)
  {
    HAL_GPIO_WritePin(xshut_ports[i], xshut_pins[i], GPIO_PIN_RESET);

    sens_online[i] = 0;
    sens_status[i] = S_NO_SENSOR;
    ptrTof_1->data[i] = D_NO_SENSOR;
  }
  vTaskDelay(pdMS_TO_TICKS(50));

  // sanity: after ALL LOW, no sensor should answer on default address
  {
    uint16_t sid = 0;
    int st0 = VL53L4CD_GetSensorId(DEFAULT_I2C_ADDRESS, &sid);
   /* printf("I2C%u sanity: after ALL LOW -> st=%d sid=0x%04X\r\n",
           (unsigned)(bus+1), st0, (unsigned)sid);*/

    if (st0 == 0 && sid == 0xEBAA) {
      /*printf("I2C%u FATAL: sensor answers while ALL XSHUT LOW (XSHUT wiring/power issue)\r\n",
             (unsigned)(bus+1));*/
      // disable bus
      switch(bus) {
        case BUS_I2C1: v_I2C1_Bus = false; break;
        case BUS_I2C2: v_I2C2_Bus = false; break;
        case BUS_I2C3: v_I2C3_Bus = false; break;
        case BUS_I2C4: v_I2C4_Bus = false; break;
      }
      return;
    }
  }

  // 2) bring up one-by-one
  for (uint8_t i = start; i < end; i++)
  {
    // قبل از بالا آوردن سنسور جدید، مطمئن شو هیچکس روی DEFAULT فعال نیست
    {
      uint16_t sid = 0;
      int st0 = VL53L4CD_GetSensorId(DEFAULT_I2C_ADDRESS, &sid);
      if (st0 == 0 && sid == 0xEBAA) {
        /*printf("I2C%u COLLISION: DEFAULT addr already responds BEFORE i=%u (XSHUT not isolating)\r\n",
               (unsigned)(bus+1), (unsigned)i);*/
        break; // ادامه نده چون آدرس‌دهی قاطی می‌شود
      }
    }

    // روشن کردن فقط همین سنسور
    HAL_GPIO_WritePin(xshut_ports[i], xshut_pins[i], GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(80));

   // printf("I2C%u bringup i=%u (XSHUT HIGH)\r\n", (unsigned)(bus+1), (unsigned)i);

    // detect on default addr
    sensor_id = 0;
    st = 1;
    for (int r=0; r<3; r++) {
      st = VL53L4CD_GetSensorId(DEFAULT_I2C_ADDRESS, &sensor_id);
      if (st == 0) break;
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  /*  printf("I2C%u i=%u GetID DEFAULT st=%d sid=0x%04X\r\n",
           (unsigned)(bus+1), (unsigned)i, st, (unsigned)sensor_id);*/


    if (st != 0 || sensor_id != 0xEBAA) {
      // not detected
      sens_online[i] = 0;
      sens_status[i] = S_NO_SENSOR;
      ptrTof_1->data[i] = D_NO_SENSOR;
      continue;
    }

    // init
    st = VL53L4CD_SensorInit(DEFAULT_I2C_ADDRESS);
   // printf("I2C%u i=%u SensorInit st=%d\r\n", (unsigned)(bus+1), (unsigned)i, st);

    if (st != 0) {
      sens_online[i] = 0;
      sens_status[i] = S_NO_SENSOR;
      ptrTof_1->data[i] = D_NO_SENSOR;
      continue;
    }

    // set new address
    st = VL53L4CD_SetI2CAddress(DEFAULT_I2C_ADDRESS, dev[i]);
   /* printf("I2C%u i=%u SetAddr DEFAULT(0x%02X)->NEW(0x%02X) st=%d\r\n",
           (unsigned)(bus+1), (unsigned)i,
           (unsigned)DEFAULT_I2C_ADDRESS, (unsigned)dev[i], st);*/

    if (st != 0) {
      sens_online[i] = 0;
      sens_status[i] = S_NO_SENSOR;
      ptrTof_1->data[i] = D_NO_SENSOR;
      continue;
    }

    // verify at new address
    sensor_id = 0;
    st = VL53L4CD_GetSensorId(dev[i], &sensor_id);
   /* printf("I2C%u i=%u Verify NEW addr 0x%02X -> st=%d sid=0x%04X %s\r\n",
           (unsigned)(bus+1), (unsigned)i, (unsigned)dev[i],
           st, (unsigned)sensor_id,
           (st==0 && sensor_id==0xEBAA) ? "OK" : "FAIL");*/

    if (st != 0 || sensor_id != 0xEBAA) {
      sens_online[i] = 0;
      sens_status[i] = S_NO_SENSOR;
      ptrTof_1->data[i] = D_NO_SENSOR;
      continue;
    }

    // start ranging
    st = VL53L4CD_StartRanging(dev[i]);
   /* printf("I2C%u i=%u StartRanging addr=0x%02X st=%d\r\n",
           (unsigned)(bus+1), (unsigned)i, (unsigned)dev[i], st);*/

    if (st != 0) {
      sens_online[i] = 0;
      sens_status[i] = S_NO_SENSOR;
      ptrTof_1->data[i] = D_NO_SENSOR;
      continue;
    }

    // OK
    sens_online[i] = 1;
    sens_status[i] = S_OK;
    g_everOnline[i] = 1;
    g_errStreak[i]  = 0;
    g_lastGoodMs[i] = HAL_GetTick();
    ok++;
  }

  bool enabled = (ok > 0);
  switch(bus) {
    case BUS_I2C1: v_I2C1_Bus = enabled; break;
    case BUS_I2C2: v_I2C2_Bus = enabled; break;
    case BUS_I2C3: v_I2C3_Bus = enabled; break;
    case BUS_I2C4: v_I2C4_Bus = enabled; break;
  }

  printf("BUS I2C%u init: OK=%u/%u -> %s\r\n",
         (unsigned)(bus+1), (unsigned)ok, (unsigned)(end-start),
         enabled ? "ENABLED":"DISABLED");
}
//===================================================
// ================== Generic READ ==================
//===================================================
static void ReadBus_Generic(
  bus_id_t bus,
  GPIO_TypeDef *ledPort, uint16_t ledPin,
  uint8_t start, uint8_t end,
  const char *tag
)
{
  // ================= BUS ENABLE CHECK =================
  bool enabled = false;
  switch(bus) {
    case BUS_I2C1: enabled = v_I2C1_Bus; break;
    case BUS_I2C2: enabled = v_I2C2_Bus; break;
    case BUS_I2C3: enabled = v_I2C3_Bus; break;
    case BUS_I2C4: enabled = v_I2C4_Bus; break;
  }

  // اگر کل باس down باشد → همه را invalid کن
  if (!enabled) {
    for (uint8_t i=start;i<end;i++) {
      ptrTof_1->data[i] = D_NO_SENSOR;
    }
    return;
  }

  const uint32_t now = HAL_GetTick();

  // ================= THRESHOLDS =================
  const uint32_t TIMEOUT_MS          = 200;    // timeout ساده
  const uint32_t NO_DATA_RESTART_MS  = 2000;   // restart ranging
  const uint32_t NO_DATA_RESET_MS    = 10000;  // power-cycle

  SetBusHandle(bus);
  I2C_HandleTypeDef *hi2c = GetBusHi2c(bus);


  // ================= LOOP SENSORS =================
  for (uint8_t i=start;i<end;i++)
  {
    // ================= OFFLINE HANDLING =================
    if (sens_online[i] == 0)
    {
      ptrTof_1->data[i] = g_everOnline[i] ? D_DISCONNECTED : D_NO_SENSOR;
      continue;
    }

    if (g_lastGoodMs[i] == 0) g_lastGoodMs[i] = now;

    uint8_t is_ready = 0;
    int st = VL53L4CD_CheckForDataReady(dev[i], &is_ready);

    // ================= I2C ERROR =================
    if (st != 0)
    {
    	g_errStreak[i]++;

    	// NOTE:
    	// چاپ خطای I2C محدود شده تا UART و CPU درگیر نشوند.
    	// فقط هر 10 خطای متوالی یک بار چاپ می‌کنیم.
    	if ((g_errStreak[i] % 10U) == 0U)
    	{
    	    printf("%s ERR: i=%u halerr=0x%lX\r\n",
    	           tag, i, (unsigned long)HAL_I2C_GetError(hi2c));
    	}  // 🔴 استفاده از hi2c


      sens_status[i] = S_I2C_ERROR;
      ptrTof_1->data[i] = D_I2C_ERROR;
      continue;
    }

    // ================= DATA READY =================
    if (is_ready)
    {
      HAL_GPIO_TogglePin(ledPort, ledPin);

      st = VL53L4CD_GetResult(dev[i], &results[i]);

      if (st == 0 && results[i].range_status == 0)
      {
        uint16_t raw = results[i].distance_mm;
        int32_t val;
        g_errStreak[i] = 0;
        // ---------- RANGE CHECK ----------
        if (raw == 0 || raw > 200)
        {
          sens_status[i] = S_I2C_ERROR;
          ptrTof_1->data[i] = D_I2C_ERROR;
          continue;
        }

        if (raw < 5)      val = 0;
        else if (raw > 50) val = 50;
        else               val = raw;

        // ---------- FILTER INITIALIZATION ----------
        // NOTE:
        // اولین نمونه معتبر نباید low-pass شود.
        // اگر از صفر شروع کنیم، مقدار 50 تبدیل به 12 می‌شود
        // و بعد spike-removal آن را اشتباه قفل می‌کند.
        if (!g_filterInitialized[i])
        {
            g_lastFiltered[i] = val;
            ptrTof_1->data[i] = val;
            g_filterInitialized[i] = 1;
        }
        else
        {
            // ---------- SPIKE REMOVAL ----------
            int32_t prev = g_lastFiltered[i];

            // NOTE:
            // اگر پرش خیلی شدید باشد، آن را spike فرض می‌کنیم
            // و مقدار قبلی را نگه می‌داریم.
            // NOTE:
            // Spike removal فقط افت ناگهانی را محدود می‌کند.
            // برگشت به فاصله بیشتر باید آزاد باشد، چون وقتی فشار برداشته می‌شود
            // مقدار باید بتواند دوباره بالا برود.
         /*   if ((prev - val) > 20)
            {
                val = prev;
            }*/

            // ---------- LOW PASS FILTER ----------
            // NOTE:
            // فیلتر سبک برای نرم کردن تغییرات طبیعی سنسور
            int32_t filtered = (prev * 3 + val) / 4;

            g_lastFiltered[i] = filtered;
            ptrTof_1->data[i] = filtered;
        }

        sens_status[i] = S_OK;
        g_lastGoodMs[i] = now;
      }

      VL53L4CD_ClearInterrupt(dev[i]);
    }
    else
    {
      // ================= NO DATA =================
      VL53L4CD_ClearInterrupt(dev[i]);

      uint32_t dt = now - g_lastGoodMs[i];
      g_errStreak[i]++;

      // ---------- SHORT TIMEOUT ----------
      if (dt > TIMEOUT_MS)
      {
        sens_status[i] = S_STALLED;
        ptrTof_1->data[i] = D_STALLED;
      }

      // ---------- RESTART RANGING ----------
      if (dt > NO_DATA_RESTART_MS && dt <= NO_DATA_RESET_MS)
      {
        VL53L4CD_StopRanging(dev[i]);
        VL53L4CD_StartRanging(dev[i]);
        g_lastGoodMs[i] = now;
      }

      // ---------- HARD RESET ----------
      if (dt > NO_DATA_RESET_MS)
      {
        int r = ResetAndReinitOne(bus, i);

        if (r == 0)
        {
          sens_status[i] = S_OK;
          g_lastGoodMs[i] = now;
        }
        else
        {
          sens_online[i] = 0;
          sens_status[i] = S_DISCONNECTED;
          ptrTof_1->data[i] = D_DISCONNECTED;
        }
      }

      if (g_errStreak[i] % 10 == 0)
      {
    	    if ((now - g_lastBusRecoverMs[bus]) > 2000)   // 🔴 throttle: هر 2 ثانیه یکبار
    	    {
    	        RecoverBus(bus);
    	        g_lastBusRecoverMs[bus] = now;            // 🔴 زمان آخرین recover
    	    }
      }
    }
  }
}
// ================== Bus-specific wrappers ==================
void VL53L4CD_Init_Multi_I2C1(void){ InitBus_Generic(BUS_I2C1, 0, 8);  }
void VL53L4CD_Init_Multi_I2C2(void){ InitBus_Generic(BUS_I2C2, 8, 16); }
void VL53L4CD_Init_Multi_I2C3(void){ InitBus_Generic(BUS_I2C3, 16, 24);}
void VL53L4CD_Init_Multi_I2C4(void){ InitBus_Generic(BUS_I2C4, 24, 32);}

void VL53L4CD_SensorRead_I2C1(void)
{
  ReadBus_Generic(BUS_I2C1, LED3_GPIO_Port, LED3_Pin, 0, 8, "I2C1");
}

void VL53L4CD_SensorRead_I2C2(void)
{
  ReadBus_Generic(BUS_I2C2, LED4_GPIO_Port, LED4_Pin, 8, 16, "I2C2");
}

void VL53L4CD_SensorRead_I2C3(void)
{
  ReadBus_Generic(BUS_I2C3, LED5_GPIO_Port, LED5_Pin, 16, 24, "I2C3");
}

void VL53L4CD_SensorRead_I2C4(void)
{
  ReadBus_Generic(BUS_I2C4, LED6_GPIO_Port, LED6_Pin, 24, 32, "I2C4");
}
