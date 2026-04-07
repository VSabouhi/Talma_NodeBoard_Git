#ifndef INC_CAN_APP_H_
#define INC_CAN_APP_H_
/*----------------------------------------------------------------------------*/
#include "can.h"
#include "FreeRTOS.h"
#include "queue.h"     // ⭐ QueueHandle_t اینجاستداری
/*----------------------------------------------------------------------------*/

typedef struct
{
    uint32_t id;      // هم Std هم Ext جا می‌شود
    uint8_t  dlc;
    uint8_t  flags;   // IDE + RTR
    uint8_t  data[8];

} CAN_Frame_t;

extern QueueHandle_t canRxQueue;
extern volatile uint32_t can_rx_overflow;

/*----------------------------------------------------------------------------*/
#define CAN_MSG_STD   0x00
#define CAN_MSG_EXT   0x01
#define CAN_MSG_RTR   0x02
/*----------------------------------------------------------------------------*/

void CAN_AppInit(void);
void CAN_AppStart(void);    // Start CAN + RX interrupt
/*----------------------------------------------------------------------------*/

#endif /* INC_CAN_APP_H_ */
