#include "can_app.h"
#include <string.h>
#include "cmsis_os.h"
#include "main.h"

/*----------------------------------------------------------------------------*/
QueueHandle_t canRxQueue = NULL;
volatile uint32_t can_rx_overflow = 0;
/*----------------------------------------------------------------------------*/
void CAN_AppInit(void)
{
    canRxQueue = xQueueCreate(256, sizeof(CAN_Frame_t));

    if(canRxQueue == NULL)
        Error_Handler();
}

/*----------------------------------------------------------------------------*/

static void CAN_FilterAcceptAll(void)
{
  CAN_FilterTypeDef f = {0};

  f.FilterBank = 0;
  f.FilterMode = CAN_FILTERMODE_IDMASK;
  f.FilterScale = CAN_FILTERSCALE_32BIT;

  // ID/MASK = 0 یعنی همه پیام‌ها match می‌شوند
  f.FilterIdHigh = 0x0000;
  f.FilterIdLow  = 0x0000;
  f.FilterMaskIdHigh = 0x0000;
  f.FilterMaskIdLow  = 0x0000;

  f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  f.FilterActivation = ENABLE;

  // اگر CAN2 نداری، همین مقدار اوکیه
  f.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan1, &f) != HAL_OK)
    Error_Handler();
}
/*----------------------------------------------------------------------------*/
void CAN_AppStart(void)
{
	CAN_FilterAcceptAll();

  if (HAL_CAN_Start(&hcan1) != HAL_OK) Error_Handler();

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    Error_Handler();
}
/*----------------------------------------------------------------------------*/
// RX ISR callback
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  BaseType_t hpw = pdFALSE;
  CAN_RxHeaderTypeDef rxh;
  CAN_Frame_t msg;

  if (hcan->Instance != CAN1) return;

  while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0)
  {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxh, msg.data) != HAL_OK)
      break;

    msg.dlc = rxh.DLC;
    msg.flags = 0;

    if (rxh.IDE == CAN_ID_EXT) { msg.flags |= CAN_MSG_EXT; msg.id = rxh.ExtId; }
    else                      { msg.id = rxh.StdId; }

    if (rxh.RTR == CAN_RTR_REMOTE) msg.flags |= CAN_MSG_RTR;

    if (xQueueSendFromISR(canRxQueue, &msg, &hpw) != pdPASS)
      can_rx_overflow++;
  }

  portYIELD_FROM_ISR(hpw);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/





