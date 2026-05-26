#include "can_node.h"
#include "main.h"
#include "can.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "v_VL53L4CD.h"
#include "v_Sensor.h"
#include "task.h"
#include "i2c.h"
#include "queue.h"
#include "platform.h"
#include <stdio.h>
#include "utils.h"
#include "v_VL53L4CD.h"
#include "sensor_health.h"   // برای دسترسی به health سنسورها
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
// ===== تنظیمات نود =====
#define BOARD_ID        1
#define TX_PERIOD_MS    1000
#define BASE_ID         0x100
// CAN ID layout:
// Sensor data  : BASE_ID        + (BOARD_ID << 2) + chunk
// Sensor health: HEALTH_BASE_ID + (BOARD_ID << 2) + chunk
#define HEALTH_BASE_ID  0x200
/*----------------------------------------------------------------------------*/
extern QueueHandle_t qSensors;
// ===== داده نود =====
static uint8_t sensors[32];
static uint8_t sensor_health_payload[32];   // بایت health هر سنسور برای ارسال روی CAN
static uint8_t sensors_packed[32];   // بایت نهایی هر سنسور برای ارسال روی CAN
/*----------------------------------------------------------------------------*/
static HAL_StatusTypeDef CAN_SendChunk(uint16_t id, const uint8_t *data8);
//static void FillSensors_MixRealAndFake(void);
static void SendAll4Chunks(void);
/*----------------------------------------------------------------------------*/

// تبدیل status داخلی سنسور به status دو بیتی برای CAN
// bit7..6:
// 00 = OK
// 01 = warning / stalled
// 10 = error / recovering
// 11 = missing / disconnected
static uint8_t pack_sensor_status_2bit(sens_status_t st)
{
    switch (st)
    {
        case S_OK:
            return 0u;   // 00

        case S_STALLED:
            return 1u;   // 01

        case S_I2C_ERROR:
        case S_RECOVERING:
            return 2u;   // 10

        case S_NO_SENSOR:
        case S_DISCONNECTED:
        default:
            return 3u;   // 11
    }
}
/*----------------------------------------------------------------------------*/

extern QueueHandle_t qI2C1;
/*----------------------------------------------------------------------------*/

static HAL_StatusTypeDef CAN_SendChunk(uint16_t id, const uint8_t *data8)
{
    CAN_TxHeaderTypeDef txh = {0};
    uint32_t mb;

    txh.IDE = CAN_ID_STD;
    txh.RTR = CAN_RTR_DATA;
    txh.StdId = id;
    txh.DLC = 8;
    txh.TransmitGlobalTime = DISABLE;

    // ⭐ فقط وقتی mailbox پر است صبر کن
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
    {
        vTaskDelay(1);   // فقط 1 tick
    }

    return HAL_CAN_AddTxMessage(&hcan1, &txh, (uint8_t*)data8, &mb);
}
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/

// تبدیل health داخلی سنسور به یک بایت قابل ارسال روی CAN
// bit0 = valid
// bit1 = load/body detected
// bit2 = noisy
// bit3 = no_update
// bit4 = stuck
// bit5 = fault
// bit6..7 = reserved
static uint8_t PackHealthByte(const sensor_health_t *h)
{
    uint8_t b = 0;

    if (h->is_valid)     b |= (1u << 0);
    if (h->is_loaded)    b |= (1u << 1);
    if (h->is_noisy)     b |= (1u << 2);
    if (h->is_no_update) b |= (1u << 3);
    if (h->is_stuck)     b |= (1u << 4);
    if (h->fault)        b |= (1u << 5);

    return b;
}
/*----------------------------------------------------------------------------*/
/*
Packed CAN sensor byte format:
bit7..6 = status
    00 -> OK
    01 -> STALLED / warning
    10 -> I2C_ERROR / RECOVERING
    11 -> NO_SENSOR / DISCONNECTED

bit5..0 = sensor value (0..63)
*/
/*----------------------------------------------------------------------------*/

static void SendAll4Chunks(void)
{
  uint16_t id0 = BASE_ID + (BOARD_ID << 2) + 0;
  uint16_t id1 = BASE_ID + (BOARD_ID << 2) + 1;
  uint16_t id2 = BASE_ID + (BOARD_ID << 2) + 2;
  uint16_t id3 = BASE_ID + (BOARD_ID << 2) + 3;

  // برای هر سنسور:
  // بیت پایین 6 = مقدار سنسور
  // بیت بالا 2  = وضعیت سنسور
  for (int i = 0; i < 32; i++)
  {
    uint8_t value6;
    uint8_t st2;

    // وضعیت سنسور را به قالب 2 بیتی تبدیل می‌کنیم
    st2 = pack_sensor_status_2bit(sens_status[i]);

    // Clamp open/no-contact range.
    //
    // NOTE:
    // Main/UI currently use this semantic:
    //   50 = no pressure / no contact
    //    0 = maximum pressure
    //
    // Values above 50 are treated as open/no-contact.
    // Do NOT invert the value here.
    // Do NOT change Main/UI pressure semantics here.
    if (sensors[i] > 50u)
    {
        value6 = 50u;
    }
    else
    {
        value6 = sensors[i];
    }

    // چیدن status و value داخل یک بایت
    sensors_packed[i] = (uint8_t)((st2 << 6) | (value6 & 0x3Fu));
  }

  // ارسال 32 سنسور در 4 فریم 8 بایتی
  (void)CAN_SendChunk(id0, &sensors_packed[0]);
  (void)CAN_SendChunk(id1, &sensors_packed[8]);
  (void)CAN_SendChunk(id2, &sensors_packed[16]);
  (void)CAN_SendChunk(id3, &sensors_packed[24]);
}


static void SendHealth4Chunks(void)
{
    uint16_t id0 = HEALTH_BASE_ID + (BOARD_ID << 2) + 0;
    uint16_t id1 = HEALTH_BASE_ID + (BOARD_ID << 2) + 1;
    uint16_t id2 = HEALTH_BASE_ID + (BOARD_ID << 2) + 2;
    uint16_t id3 = HEALTH_BASE_ID + (BOARD_ID << 2) + 3;

    const sensor_health_t *h = SensorHealth_GetAll();

    // ساخت payload سلامت برای 32 سنسور
    // NOTE:
    // هر سنسور یک بایت health دارد.
    // ترتیب health دقیقاً مثل ترتیب sensor data است.
    for (int i = 0; i < 32; i++)
    {
        sensor_health_payload[i] = PackHealthByte(&h[i]);
    }

    // ارسال health در 4 فریم 8 بایتی
    (void)CAN_SendChunk(id0, &sensor_health_payload[0]);
    (void)CAN_SendChunk(id1, &sensor_health_payload[8]);
    (void)CAN_SendChunk(id2, &sensor_health_payload[16]);
    (void)CAN_SendChunk(id3, &sensor_health_payload[24]);
}
/*----------------------------------------------------------------------------*/

// ===== API ها =====
void CAN_NodeInit(void)
{
  //CAN_FilterAcceptAll();   // فعلاً برای تست

  vTaskDelay(pdMS_TO_TICKS(300));
  printf("NODE start. IDs: 0x%03X..0x%03X\r\n",
         BASE_ID + (BOARD_ID<<2),
         BASE_ID + (BOARD_ID<<2) + 3);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

void CAN_NodeTxTask(void *argument)
{
    vTaskDelay(pdMS_TO_TICKS(300));

    tof_payload_t pkt;
    TickType_t last = xTaskGetTickCount();

    // مقدار اولیه
    memset(sensors, 0, sizeof(sensors));

    for (;;)
    {
        // ارسال دوره‌ای دقیق
        vTaskDelayUntil(&last, pdMS_TO_TICKS(1000));

        // آخرین دیتا را بگیر (اگر نبود، همان قبلی ارسال می‌شود)
        if (xQueueReceive(qSensors, &pkt, 0) == pdPASS)
        {
            memcpy(sensors, pkt.d, 32);
        }

        SendAll4Chunks();

        // ارسال health سنسورها بعد از data
        // NOTE:
        // Main Board با این فریم‌ها می‌فهمد کدام data معتبر، loaded، noisy یا fault است.
        SendHealth4Chunks();
    }
}


/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
