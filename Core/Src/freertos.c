/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_app.h"
#include "can_node.h"
#include "v_VL53L4CD.h"
#include "platform.h"
#include "i2c.h"
#include "v_Sensor.h"
#include "utils.h"
#include "queue.h"
#include <stdio.h>
#include "stepperMotor.h"
#include "motor_cmd.h"
#include <stdio.h>
#include "sensor_baseline.h"
#include "sensor_health.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
//2
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SENSOR_QUEUE_LEN        1
#define DEBUG_TASK_STACK_WORDS  256
#define NODETX_TASK_STACK_WORDS 512
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t qSensors = NULL;
extern QueueHandle_t canRxQueue;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for v_SensorTask */
osThreadId_t v_SensorTaskHandle;
uint32_t SensorTaskBuffer[ 1024 ];
osStaticThreadDef_t SensorTaskControlBlock;
const osThreadAttr_t v_SensorTask_attributes = {
  .name = "v_SensorTask",
  .cb_mem = &SensorTaskControlBlock,
  .cb_size = sizeof(SensorTaskControlBlock),
  .stack_mem = &SensorTaskBuffer[0],
  .stack_size = sizeof(SensorTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for v_CmdTask */
osThreadId_t v_CmdTaskHandle;
uint32_t v_CmdTaskBuffer[ 1024 ];
osStaticThreadDef_t v_CmdTaskControlBlock;
const osThreadAttr_t v_CmdTask_attributes = {
  .name = "v_CmdTask",
  .cb_mem = &v_CmdTaskControlBlock,
  .cb_size = sizeof(v_CmdTaskControlBlock),
  .stack_mem = &v_CmdTaskBuffer[0],
  .stack_size = sizeof(v_CmdTaskBuffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for v_CanTxTask */
osThreadId_t v_CanTxTaskHandle;
uint32_t v_CanTxTaskBuffer[ 512 ];
osStaticThreadDef_t v_CanTxTaskControlBlock;
const osThreadAttr_t v_CanTxTask_attributes = {
  .name = "v_CanTxTask",
  .cb_mem = &v_CanTxTaskControlBlock,
  .cb_size = sizeof(v_CanTxTaskControlBlock),
  .stack_mem = &v_CanTxTaskBuffer[0],
  .stack_size = sizeof(v_CanTxTaskBuffer),
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void DebugTask(void *argument);
void CAN_NodeTxTask(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void SensorTask(void *argument);
void CmdTask(void *argument);
void CanTxTask(void *argument);
void print_can_frame(const CAN_Frame_t* f);


void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	CAN_AppInit();
	CAN_AppStart();
	stepper_init();
	// مقداردهی اولیه مانیتور سلامت سنسورها
	// NOTE:
	// این فقط history داخلی health را پاک می‌کند.
	// baseline جداگانه بعد از init سنسورها شروع می‌شود.
	SensorHealth_Init();

	qSensors = xQueueCreate(1, sizeof(tof_payload_t));
	if(qSensors == NULL)  Error_Handler();


	  if (xTaskCreate(DebugTask,
	                  "DBG",
	                  DEBUG_TASK_STACK_WORDS,
	                  NULL,
	                  tskIDLE_PRIORITY + 1,
	                  NULL) != pdPASS) {
	    Error_Handler();
	  }
	  if (xTaskCreate(CAN_NodeTxTask,
	                    "NodeTX",
	                    NODETX_TASK_STACK_WORDS,
	                    NULL,
	                    tskIDLE_PRIORITY + 2,
	                    NULL) != pdPASS) {
	      Error_Handler();
	    }
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of v_SensorTask */
  v_SensorTaskHandle = osThreadNew(SensorTask, NULL, &v_SensorTask_attributes);

  /* creation of v_CmdTask */
  v_CmdTaskHandle = osThreadNew(CmdTask, NULL, &v_CmdTask_attributes);

  /* creation of v_CanTxTask */
  v_CanTxTaskHandle = osThreadNew(CanTxTask, NULL, &v_CanTxTask_attributes);

  if ((defaultTaskHandle == NULL) ||
      (v_SensorTaskHandle == NULL) ||
      (v_CmdTaskHandle == NULL) ||
      (v_CanTxTaskHandle == NULL)) {
    Error_Handler();
  }

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {

	// پردازش non-blocking baseline

    osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_SensorTask */
/**
* @brief Function implementing the v_SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SensorTask */
void SensorTask(void *argument)
{
  (void)argument;

	#if DBG_BUS_STATUS
		TickType_t lastPrint = 0;   // NOTE: فقط وقتی debug باس فعال است استفاده می‌شود
	#endif

  tof_payload_t pkt;

  memset(&pkt, 0, sizeof(pkt));

  initialize_ptrTof_1_data();

  /* ---------------- Multi-bus sensor init ---------------- */
  VL53L4CD_SetI2CHandle(&hi2c1);
  VL53L4CD_Init_Multi_I2C1();

  VL53L4CD_SetI2CHandle(&hi2c2);
  VL53L4CD_Init_Multi_I2C2();

  VL53L4CD_SetI2CHandle(&hi2c3);
  VL53L4CD_Init_Multi_I2C3();

  VL53L4CD_SetI2CHandle(&hi2c4);
  VL53L4CD_Init_Multi_I2C4();

  // شروع baseline بعد از init کامل سنسورها
  // NOTE: اینجا سنسورها address گرفته‌اند و آماده read هستند
  SensorBaseline_Start();


  printf("SensorTask started\r\n");

  for (;;)
  {
    /* فقط باس‌های فعال خوانده شوند */
    if (v_I2C1_Bus) VL53L4CD_SensorRead_I2C1();
    if (v_I2C2_Bus) VL53L4CD_SensorRead_I2C2();
    if (v_I2C3_Bus) VL53L4CD_SensorRead_I2C3();
    if (v_I2C4_Bus) VL53L4CD_SensorRead_I2C4();

    // پردازش baseline بعد از به‌روزرسانی data[]
    // NOTE:
    // baseline باید بعد از SensorRead اجرا شود،
    // چون از ptrTof_1->data[] و sens_status[] نمونه می‌گیرد.
    SensorBaseline_Process();
    if (SensorBaseline_IsDone())
    {
        SensorHealth_Process();
    }
    /* مستقیم از data[] پکت بساز */
    for (uint8_t i = 0; i < 32; i++) {
      pkt.d[i] = clamp_u8(ptrTof_1->data[i]);
    }

    xQueueOverwrite(qSensors, &pkt);

    vTaskDelay(pdMS_TO_TICKS(100));

	#if DBG_BUS_STATUS

			if ((xTaskGetTickCount() - lastPrint) > pdMS_TO_TICKS(1000))
			{
				lastPrint = xTaskGetTickCount();

				printf("BUS EN: I2C1=%u I2C2=%u I2C3=%u I2C4=%u | DATA: ",
					   v_I2C1_Bus,
					   v_I2C2_Bus,
					   v_I2C3_Bus,
					   v_I2C4_Bus);

				for (int i = 0; i < 8; i++) {
					printf("%u ", pkt.d[i]);
				}

				printf("\r\n");
			}

	#endif
  }
}


/* USER CODE BEGIN Header_CmdTask */
/**
* @brief Function implementing the v_CmdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CmdTask */
void CmdTask(void *argument)
{
  const uint8_t BOARD_ID = 1;
  CAN_Frame_t f;

  for (;;)
  {
    if (xQueueReceive(canRxQueue, &f, portMAX_DELAY) == pdPASS)
    {
     // HAL_GPIO_TogglePin(LED7_GPIO_Port, LED7_Pin);

      // اگر خواستی چاپ هم بکن
      // printf("RX id=0x%lX dlc=%u data0=0x%02X\r\n", f.id, f.dlc, f.data[0]);
      print_can_frame(&f);
      motor_cmd_dispatch(&f, BOARD_ID);
    }
  }
}

/* USER CODE BEGIN Header_CanTxTask */
/**
* @brief Function implementing the v_CanTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CanTxTask */
void CanTxTask(void *argument)
{
  /* USER CODE BEGIN CanTxTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END CanTxTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void DebugTask(void *argument)
{

    while(1)
    {
        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

	#if DBG_HEALTH


		if (SensorBaseline_IsDone())
		{
			const sensor_baseline_data_t *b = SensorBaseline_GetData();

			// گرفتن health فعلی سنسورها
			// NOTE: فقط برای دیباگ چاپ استفاده می‌شود
			const sensor_health_t *h = SensorHealth_GetAll();

			printf("HEALTH:\r\n");

			// چاپ سلامت فقط برای سنسورهایی که baseline معتبر دارند
			// NOTE: سنسورهای وصل‌نشده یا بدون baseline چاپ نمی‌شوند
			for (int i = 0; i < 32; i++)
			{
				if (b->valid[i])
				{
					printf("H[%02d] valid=%u load=%u noisy=%u no_upd=%u stuck=%u fault=%u\r\n",
						   i,
						   h[i].is_valid,
						   h[i].is_loaded,
						   h[i].is_noisy,
						   h[i].is_no_update,
						   h[i].is_stuck,
						   h[i].fault);
				}
			}

		}

	#endif

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
/*----------------------------------------------------------------------------*/

 void print_can_frame(const CAN_Frame_t* f)
{
  printf("CAN RX: id=0x%lX dlc=%u flags=0x%02X data=",
         (unsigned long)f->id, f->dlc, f->flags);

  for (int i = 0; i < f->dlc; i++)
    printf("%02X ", f->data[i]);

  printf("\r\n");
}
/* USER CODE END Application */

