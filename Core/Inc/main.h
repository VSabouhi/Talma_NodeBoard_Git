/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Pix08_En_Pin GPIO_PIN_2
#define Pix08_En_GPIO_Port GPIOE
#define Pix08_Xs_Pin GPIO_PIN_3
#define Pix08_Xs_GPIO_Port GPIOE
#define Pix09_PW_Pin GPIO_PIN_4
#define Pix09_PW_GPIO_Port GPIOE
#define Pix09_Dir_Pin GPIO_PIN_5
#define Pix09_Dir_GPIO_Port GPIOE
#define Pix09_En_Pin GPIO_PIN_6
#define Pix09_En_GPIO_Port GPIOE
#define Pix09_Xs_Pin GPIO_PIN_8
#define Pix09_Xs_GPIO_Port GPIOI
#define Pix10_PW_Pin GPIO_PIN_13
#define Pix10_PW_GPIO_Port GPIOC
#define Pix10_Dir_Pin GPIO_PIN_9
#define Pix10_Dir_GPIO_Port GPIOI
#define Pix10_En_Pin GPIO_PIN_10
#define Pix10_En_GPIO_Port GPIOI
#define Pix11_Xs_Pin GPIO_PIN_11
#define Pix11_Xs_GPIO_Port GPIOI
#define Pix11_PW_Pin GPIO_PIN_2
#define Pix11_PW_GPIO_Port GPIOF
#define Pix11_Dir_Pin GPIO_PIN_12
#define Pix11_Dir_GPIO_Port GPIOI
#define Pix11_En_Pin GPIO_PIN_13
#define Pix11_En_GPIO_Port GPIOI
#define Pix11_XsI14_Pin GPIO_PIN_14
#define Pix11_XsI14_GPIO_Port GPIOI
#define Pix12_PW_Pin GPIO_PIN_3
#define Pix12_PW_GPIO_Port GPIOF
#define Pix12_Dir_Pin GPIO_PIN_4
#define Pix12_Dir_GPIO_Port GPIOF
#define Pix12_En_Pin GPIO_PIN_5
#define Pix12_En_GPIO_Port GPIOF
#define Pix12_Xs_Pin GPIO_PIN_6
#define Pix12_Xs_GPIO_Port GPIOF
#define Pix13_PW_Pin GPIO_PIN_7
#define Pix13_PW_GPIO_Port GPIOF
#define Pix13_Dir_Pin GPIO_PIN_8
#define Pix13_Dir_GPIO_Port GPIOF
#define Pix13_En_Pin GPIO_PIN_9
#define Pix13_En_GPIO_Port GPIOF
#define Pix13_Xs_Pin GPIO_PIN_10
#define Pix13_Xs_GPIO_Port GPIOF
#define Pix14_PW_Pin GPIO_PIN_0
#define Pix14_PW_GPIO_Port GPIOC
#define Pix14_Dir_Pin GPIO_PIN_1
#define Pix14_Dir_GPIO_Port GPIOC
#define Pix14_En_Pin GPIO_PIN_2
#define Pix14_En_GPIO_Port GPIOC
#define Pix14_Xs_Pin GPIO_PIN_3
#define Pix14_Xs_GPIO_Port GPIOC
#define Pix15_PW_Pin GPIO_PIN_0
#define Pix15_PW_GPIO_Port GPIOA
#define Pix15_Dir_Pin GPIO_PIN_1
#define Pix15_Dir_GPIO_Port GPIOA
#define Pix15_En_Pin GPIO_PIN_2
#define Pix15_En_GPIO_Port GPIOA
#define Pix15_Xs_Pin GPIO_PIN_2
#define Pix15_Xs_GPIO_Port GPIOH
#define Pix16_PW_Pin GPIO_PIN_3
#define Pix16_PW_GPIO_Port GPIOH
#define Pix16_Dir_Pin GPIO_PIN_4
#define Pix16_Dir_GPIO_Port GPIOH
#define Pix16_En_Pin GPIO_PIN_5
#define Pix16_En_GPIO_Port GPIOH
#define Pix16_Xs_Pin GPIO_PIN_3
#define Pix16_Xs_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_4
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_6
#define LED3_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_7
#define LED4_GPIO_Port GPIOA
#define LED5_Pin GPIO_PIN_4
#define LED5_GPIO_Port GPIOC
#define LED6_Pin GPIO_PIN_5
#define LED6_GPIO_Port GPIOC
#define LED7_Pin GPIO_PIN_0
#define LED7_GPIO_Port GPIOB
#define Pix17_Xs_Pin GPIO_PIN_2
#define Pix17_Xs_GPIO_Port GPIOB
#define Pix17_En_Pin GPIO_PIN_15
#define Pix17_En_GPIO_Port GPIOI
#define Pix17_Dir_Pin GPIO_PIN_0
#define Pix17_Dir_GPIO_Port GPIOJ
#define Pix17_PW_Pin GPIO_PIN_1
#define Pix17_PW_GPIO_Port GPIOJ
#define Pix18_Xs_Pin GPIO_PIN_2
#define Pix18_Xs_GPIO_Port GPIOJ
#define Pix18_En_Pin GPIO_PIN_3
#define Pix18_En_GPIO_Port GPIOJ
#define Pix18_Dir_Pin GPIO_PIN_4
#define Pix18_Dir_GPIO_Port GPIOJ
#define Pix18_PW_Pin GPIO_PIN_11
#define Pix18_PW_GPIO_Port GPIOF
#define Pix19_Xs_Pin GPIO_PIN_12
#define Pix19_Xs_GPIO_Port GPIOF
#define Pix19_En_Pin GPIO_PIN_13
#define Pix19_En_GPIO_Port GPIOF
#define Pix19_Dir_Pin GPIO_PIN_0
#define Pix19_Dir_GPIO_Port GPIOG
#define Pix19_PW_Pin GPIO_PIN_1
#define Pix19_PW_GPIO_Port GPIOG
#define Pix20_Xs_Pin GPIO_PIN_7
#define Pix20_Xs_GPIO_Port GPIOE
#define Pix20_En_Pin GPIO_PIN_8
#define Pix20_En_GPIO_Port GPIOE
#define Pix20_Dir_Pin GPIO_PIN_10
#define Pix20_Dir_GPIO_Port GPIOE
#define Pix20_PW_Pin GPIO_PIN_11
#define Pix20_PW_GPIO_Port GPIOE
#define Pix21_Xs_Pin GPIO_PIN_12
#define Pix21_Xs_GPIO_Port GPIOE
#define Pix21_En_Pin GPIO_PIN_13
#define Pix21_En_GPIO_Port GPIOE
#define Pix21_Dir_Pin GPIO_PIN_14
#define Pix21_Dir_GPIO_Port GPIOE
#define Pix21_PW_Pin GPIO_PIN_15
#define Pix21_PW_GPIO_Port GPIOE
#define Pix22_Xs_Pin GPIO_PIN_10
#define Pix22_Xs_GPIO_Port GPIOB
#define Pix22_En_Pin GPIO_PIN_11
#define Pix22_En_GPIO_Port GPIOB
#define Pix22_Dir_Pin GPIO_PIN_5
#define Pix22_Dir_GPIO_Port GPIOJ
#define Pix22_PW_Pin GPIO_PIN_6
#define Pix22_PW_GPIO_Port GPIOH
#define Pix23_Xs_Pin GPIO_PIN_9
#define Pix23_Xs_GPIO_Port GPIOH
#define Pix23_En_Pin GPIO_PIN_10
#define Pix23_En_GPIO_Port GPIOH
#define Pix23_Dir_Pin GPIO_PIN_11
#define Pix23_Dir_GPIO_Port GPIOH
#define Pix23_PW_Pin GPIO_PIN_12
#define Pix23_PW_GPIO_Port GPIOH
#define Pix24_Xs_Pin GPIO_PIN_12
#define Pix24_Xs_GPIO_Port GPIOB
#define Pix24_En_Pin GPIO_PIN_13
#define Pix24_En_GPIO_Port GPIOB
#define Pix24_Dir_Pin GPIO_PIN_14
#define Pix24_Dir_GPIO_Port GPIOB
#define Pix24_PW_Pin GPIO_PIN_15
#define Pix24_PW_GPIO_Port GPIOB
#define Pix24_XsD10_Pin GPIO_PIN_10
#define Pix24_XsD10_GPIO_Port GPIOD
#define Pix25_En_Pin GPIO_PIN_11
#define Pix25_En_GPIO_Port GPIOD
#define Pix25_Dir_Pin GPIO_PIN_12
#define Pix25_Dir_GPIO_Port GPIOD
#define Pix25_PW_Pin GPIO_PIN_13
#define Pix25_PW_GPIO_Port GPIOD
#define Pix26_Xs_Pin GPIO_PIN_15
#define Pix26_Xs_GPIO_Port GPIOD
#define Pix26_En_Pin GPIO_PIN_6
#define Pix26_En_GPIO_Port GPIOJ
#define Pix26_Dir_Pin GPIO_PIN_7
#define Pix26_Dir_GPIO_Port GPIOJ
#define Pix26_PW_Pin GPIO_PIN_8
#define Pix26_PW_GPIO_Port GPIOJ
#define Pix27_Xs_Pin GPIO_PIN_9
#define Pix27_Xs_GPIO_Port GPIOJ
#define Pix27_En_Pin GPIO_PIN_10
#define Pix27_En_GPIO_Port GPIOJ
#define Pix27_Dir_Pin GPIO_PIN_11
#define Pix27_Dir_GPIO_Port GPIOJ
#define Pix27_PW_Pin GPIO_PIN_0
#define Pix27_PW_GPIO_Port GPIOK
#define Pix28_Xs_Pin GPIO_PIN_1
#define Pix28_Xs_GPIO_Port GPIOK
#define Pix28_En_Pin GPIO_PIN_2
#define Pix28_En_GPIO_Port GPIOK
#define Pix28_Dir_Pin GPIO_PIN_2
#define Pix28_Dir_GPIO_Port GPIOG
#define Pix28_PW_Pin GPIO_PIN_3
#define Pix28_PW_GPIO_Port GPIOG
#define Pix29_Xs_Pin GPIO_PIN_4
#define Pix29_Xs_GPIO_Port GPIOG
#define Pix29_En_Pin GPIO_PIN_5
#define Pix29_En_GPIO_Port GPIOG
#define Pix29_Dir_Pin GPIO_PIN_6
#define Pix29_Dir_GPIO_Port GPIOG
#define Pix29_PW_Pin GPIO_PIN_7
#define Pix29_PW_GPIO_Port GPIOG
#define Pix30_Xs_Pin GPIO_PIN_8
#define Pix30_Xs_GPIO_Port GPIOG
#define Pix30_En_Pin GPIO_PIN_6
#define Pix30_En_GPIO_Port GPIOC
#define Pix30_Dir_Pin GPIO_PIN_7
#define Pix30_Dir_GPIO_Port GPIOC
#define Pix30_PW_Pin GPIO_PIN_8
#define Pix30_PW_GPIO_Port GPIOC
#define Pix31_Xs_Pin GPIO_PIN_9
#define Pix31_Xs_GPIO_Port GPIOC
#define Pix31_En_Pin GPIO_PIN_8
#define Pix31_En_GPIO_Port GPIOA
#define Pix31_Dir_Pin GPIO_PIN_9
#define Pix31_Dir_GPIO_Port GPIOA
#define Pix31_PW_Pin GPIO_PIN_10
#define Pix31_PW_GPIO_Port GPIOA
#define Pix32_Xs_Pin GPIO_PIN_11
#define Pix32_Xs_GPIO_Port GPIOA
#define Pix32_En_Pin GPIO_PIN_12
#define Pix32_En_GPIO_Port GPIOA
#define Pix32_Dir_Pin GPIO_PIN_13
#define Pix32_Dir_GPIO_Port GPIOH
#define Pix32_PW_Pin GPIO_PIN_14
#define Pix32_PW_GPIO_Port GPIOH
#define Pix33_Xs_Pin GPIO_PIN_15
#define Pix33_Xs_GPIO_Port GPIOH
#define Pix33_En_Pin GPIO_PIN_0
#define Pix33_En_GPIO_Port GPIOI
#define Pix33_Dir_Pin GPIO_PIN_1
#define Pix33_Dir_GPIO_Port GPIOI
#define Pix33_PW_Pin GPIO_PIN_2
#define Pix33_PW_GPIO_Port GPIOI
#define Pix01_PW_Pin GPIO_PIN_10
#define Pix01_PW_GPIO_Port GPIOC
#define Pix01_Dir_Pin GPIO_PIN_11
#define Pix01_Dir_GPIO_Port GPIOC
#define Pix01_En_Pin GPIO_PIN_12
#define Pix01_En_GPIO_Port GPIOC
#define Pix01_Xs_Pin GPIO_PIN_0
#define Pix01_Xs_GPIO_Port GPIOD
#define Pix02_PW_Pin GPIO_PIN_1
#define Pix02_PW_GPIO_Port GPIOD
#define Pix02_Dir_Pin GPIO_PIN_2
#define Pix02_Dir_GPIO_Port GPIOD
#define Pix02_En_Pin GPIO_PIN_3
#define Pix02_En_GPIO_Port GPIOD
#define Pix02_Xs_Pin GPIO_PIN_4
#define Pix02_Xs_GPIO_Port GPIOD
#define Pix03_PW_Pin GPIO_PIN_7
#define Pix03_PW_GPIO_Port GPIOD
#define Pix03_Dir_Pin GPIO_PIN_12
#define Pix03_Dir_GPIO_Port GPIOJ
#define Pix03_En_Pin GPIO_PIN_13
#define Pix03_En_GPIO_Port GPIOJ
#define Pix03_Xs_Pin GPIO_PIN_14
#define Pix03_Xs_GPIO_Port GPIOJ
#define Pix04_PW_Pin GPIO_PIN_15
#define Pix04_PW_GPIO_Port GPIOJ
#define Pix04_Dir_Pin GPIO_PIN_9
#define Pix04_Dir_GPIO_Port GPIOG
#define Pix04_En_Pin GPIO_PIN_10
#define Pix04_En_GPIO_Port GPIOG
#define Pix04_Xs_Pin GPIO_PIN_11
#define Pix04_Xs_GPIO_Port GPIOG
#define Pix05_PW_Pin GPIO_PIN_12
#define Pix05_PW_GPIO_Port GPIOG
#define Pix05_Dir_Pin GPIO_PIN_13
#define Pix05_Dir_GPIO_Port GPIOG
#define Pix05_En_Pin GPIO_PIN_14
#define Pix05_En_GPIO_Port GPIOG
#define Pix05_Xs_Pin GPIO_PIN_3
#define Pix05_Xs_GPIO_Port GPIOK
#define Pix06_PW_Pin GPIO_PIN_4
#define Pix06_PW_GPIO_Port GPIOK
#define Pix06_Dir_Pin GPIO_PIN_5
#define Pix06_Dir_GPIO_Port GPIOK
#define Pix06_En_Pin GPIO_PIN_6
#define Pix06_En_GPIO_Port GPIOK
#define Pix06_Xs_Pin GPIO_PIN_7
#define Pix06_Xs_GPIO_Port GPIOK
#define Pix07_PW_Pin GPIO_PIN_15
#define Pix07_PW_GPIO_Port GPIOG
#define Pix07_Dir_Pin GPIO_PIN_5
#define Pix07_Dir_GPIO_Port GPIOB
#define Pix07_En_Pin GPIO_PIN_0
#define Pix07_En_GPIO_Port GPIOE
#define Pix07_EnE1_Pin GPIO_PIN_1
#define Pix07_EnE1_GPIO_Port GPIOE
#define Pix08_PW_Pin GPIO_PIN_6
#define Pix08_PW_GPIO_Port GPIOI
#define Pix08_Dir_Pin GPIO_PIN_7
#define Pix08_Dir_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
