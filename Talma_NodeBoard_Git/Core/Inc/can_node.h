#ifndef INC_CAN_NODE_H_
#define INC_CAN_NODE_H_
/*----------------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern QueueHandle_t qI2C1;
/*----------------------------------------------------------------------------*/

void CAN_NodeInit(void);      // تنظیم CAN مربوط به نود (filter/start/...)
void CAN_NodeTxTask(void *argument);  //
/*----------------------------------------------------------------------------*/


#endif /* INC_CAN_NODE_H_ */
