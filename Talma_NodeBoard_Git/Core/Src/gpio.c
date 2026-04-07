/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, Pix08_En_Pin|Pix08_Xs_Pin|Pix09_PW_Pin|Pix09_En_Pin
                          |Pix20_Xs_Pin|Pix20_En_Pin|Pix20_PW_Pin|Pix21_Xs_Pin
                          |Pix21_En_Pin|Pix21_PW_Pin|Pix07_En_Pin|Pix07_EnE1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, Pix09_Dir_Pin|Pix20_Dir_Pin|Pix21_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, Pix09_Xs_Pin|Pix10_En_Pin|Pix11_Xs_Pin|Pix11_En_Pin
                          |Pix11_XsI14_Pin|Pix17_En_Pin|Pix33_En_Pin|Pix33_Dir_Pin
                          |Pix33_PW_Pin|Pix08_PW_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Pix10_PW_Pin|Pix14_PW_Pin|Pix14_En_Pin|Pix14_Xs_Pin
                          |LED5_Pin|LED6_Pin|Pix30_En_Pin|Pix30_PW_Pin
                          |Pix31_Xs_Pin|Pix01_PW_Pin|Pix01_En_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, Pix10_Dir_Pin|Pix11_Dir_Pin|Pix08_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, Pix11_PW_Pin|Pix12_PW_Pin|Pix12_En_Pin|Pix12_Xs_Pin
                          |Pix13_PW_Pin|Pix13_En_Pin|Pix13_Xs_Pin|Pix18_PW_Pin
                          |Pix19_Xs_Pin|Pix19_En_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, Pix12_Dir_Pin|Pix13_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Pix14_Dir_Pin|Pix30_Dir_Pin|Pix01_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Pix15_PW_Pin|Pix15_En_Pin|Pix16_Xs_Pin|LED1_Pin
                          |LED2_Pin|LED3_Pin|LED4_Pin|Pix31_En_Pin
                          |Pix31_PW_Pin|Pix32_Xs_Pin|Pix32_En_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Pix15_Dir_Pin|Pix31_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, Pix15_Xs_Pin|Pix16_PW_Pin|Pix16_En_Pin|Pix22_PW_Pin
                          |Pix23_Xs_Pin|Pix23_En_Pin|Pix23_PW_Pin|Pix32_PW_Pin
                          |Pix33_Xs_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, Pix16_Dir_Pin|Pix23_Dir_Pin|Pix32_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED7_Pin|Pix17_Xs_Pin|Pix22_Xs_Pin|Pix22_En_Pin
                          |Pix24_Xs_Pin|Pix24_En_Pin|Pix24_PW_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, Pix17_Dir_Pin|Pix18_Dir_Pin|Pix22_Dir_Pin|Pix26_Dir_Pin
                          |Pix27_Dir_Pin|Pix03_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, Pix17_PW_Pin|Pix18_Xs_Pin|Pix18_En_Pin|Pix26_En_Pin
                          |Pix26_PW_Pin|Pix27_Xs_Pin|Pix27_En_Pin|Pix03_En_Pin
                          |Pix03_Xs_Pin|Pix04_PW_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, Pix19_Dir_Pin|Pix28_Dir_Pin|Pix05_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, Pix19_PW_Pin|Pix28_PW_Pin|Pix29_Xs_Pin|Pix29_En_Pin
                          |Pix29_Dir_Pin|Pix29_PW_Pin|Pix30_Xs_Pin|Pix04_Dir_Pin
                          |Pix04_En_Pin|Pix04_Xs_Pin|Pix05_PW_Pin|Pix05_En_Pin
                          |Pix07_PW_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Pix24_Dir_Pin|Pix07_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Pix24_XsD10_Pin|Pix25_En_Pin|Pix25_PW_Pin|Pix26_Xs_Pin
                          |Pix01_Xs_Pin|Pix02_PW_Pin|Pix02_En_Pin|Pix02_Xs_Pin
                          |Pix03_PW_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Pix25_Dir_Pin|Pix02_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOK, Pix27_PW_Pin|Pix28_Xs_Pin|Pix28_En_Pin|Pix05_Xs_Pin
                          |Pix06_PW_Pin|Pix06_En_Pin|Pix06_Xs_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Pix06_Dir_GPIO_Port, Pix06_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Pix08_En_Pin Pix08_Xs_Pin Pix09_PW_Pin Pix09_Dir_Pin
                           Pix09_En_Pin Pix20_Xs_Pin Pix20_En_Pin Pix20_Dir_Pin
                           Pix20_PW_Pin Pix21_Xs_Pin Pix21_En_Pin Pix21_Dir_Pin
                           Pix21_PW_Pin Pix07_En_Pin Pix07_EnE1_Pin */
  GPIO_InitStruct.Pin = Pix08_En_Pin|Pix08_Xs_Pin|Pix09_PW_Pin|Pix09_Dir_Pin
                          |Pix09_En_Pin|Pix20_Xs_Pin|Pix20_En_Pin|Pix20_Dir_Pin
                          |Pix20_PW_Pin|Pix21_Xs_Pin|Pix21_En_Pin|Pix21_Dir_Pin
                          |Pix21_PW_Pin|Pix07_En_Pin|Pix07_EnE1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix09_Xs_Pin Pix10_Dir_Pin Pix10_En_Pin Pix11_Xs_Pin
                           Pix11_Dir_Pin Pix11_En_Pin Pix11_XsI14_Pin Pix17_En_Pin
                           Pix33_En_Pin Pix33_Dir_Pin Pix33_PW_Pin Pix08_PW_Pin
                           Pix08_Dir_Pin */
  GPIO_InitStruct.Pin = Pix09_Xs_Pin|Pix10_Dir_Pin|Pix10_En_Pin|Pix11_Xs_Pin
                          |Pix11_Dir_Pin|Pix11_En_Pin|Pix11_XsI14_Pin|Pix17_En_Pin
                          |Pix33_En_Pin|Pix33_Dir_Pin|Pix33_PW_Pin|Pix08_PW_Pin
                          |Pix08_Dir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix10_PW_Pin Pix14_PW_Pin Pix14_Dir_Pin Pix14_En_Pin
                           Pix14_Xs_Pin LED5_Pin LED6_Pin Pix30_En_Pin
                           Pix30_Dir_Pin Pix30_PW_Pin Pix31_Xs_Pin Pix01_PW_Pin
                           Pix01_Dir_Pin Pix01_En_Pin */
  GPIO_InitStruct.Pin = Pix10_PW_Pin|Pix14_PW_Pin|Pix14_Dir_Pin|Pix14_En_Pin
                          |Pix14_Xs_Pin|LED5_Pin|LED6_Pin|Pix30_En_Pin
                          |Pix30_Dir_Pin|Pix30_PW_Pin|Pix31_Xs_Pin|Pix01_PW_Pin
                          |Pix01_Dir_Pin|Pix01_En_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix11_PW_Pin Pix12_PW_Pin Pix12_Dir_Pin Pix12_En_Pin
                           Pix12_Xs_Pin Pix13_PW_Pin Pix13_Dir_Pin Pix13_En_Pin
                           Pix13_Xs_Pin Pix18_PW_Pin Pix19_Xs_Pin Pix19_En_Pin */
  GPIO_InitStruct.Pin = Pix11_PW_Pin|Pix12_PW_Pin|Pix12_Dir_Pin|Pix12_En_Pin
                          |Pix12_Xs_Pin|Pix13_PW_Pin|Pix13_Dir_Pin|Pix13_En_Pin
                          |Pix13_Xs_Pin|Pix18_PW_Pin|Pix19_Xs_Pin|Pix19_En_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix15_PW_Pin Pix15_Dir_Pin Pix15_En_Pin Pix16_Xs_Pin
                           LED1_Pin LED2_Pin LED3_Pin LED4_Pin
                           Pix31_En_Pin Pix31_Dir_Pin Pix31_PW_Pin Pix32_Xs_Pin
                           Pix32_En_Pin */
  GPIO_InitStruct.Pin = Pix15_PW_Pin|Pix15_Dir_Pin|Pix15_En_Pin|Pix16_Xs_Pin
                          |LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin
                          |Pix31_En_Pin|Pix31_Dir_Pin|Pix31_PW_Pin|Pix32_Xs_Pin
                          |Pix32_En_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix15_Xs_Pin Pix16_PW_Pin Pix16_Dir_Pin Pix16_En_Pin
                           Pix22_PW_Pin Pix23_Xs_Pin Pix23_En_Pin Pix23_Dir_Pin
                           Pix23_PW_Pin Pix32_Dir_Pin Pix32_PW_Pin Pix33_Xs_Pin */
  GPIO_InitStruct.Pin = Pix15_Xs_Pin|Pix16_PW_Pin|Pix16_Dir_Pin|Pix16_En_Pin
                          |Pix22_PW_Pin|Pix23_Xs_Pin|Pix23_En_Pin|Pix23_Dir_Pin
                          |Pix23_PW_Pin|Pix32_Dir_Pin|Pix32_PW_Pin|Pix33_Xs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : LED7_Pin Pix17_Xs_Pin Pix22_Xs_Pin Pix22_En_Pin
                           Pix24_Xs_Pin Pix24_En_Pin Pix24_Dir_Pin Pix24_PW_Pin
                           Pix07_Dir_Pin */
  GPIO_InitStruct.Pin = LED7_Pin|Pix17_Xs_Pin|Pix22_Xs_Pin|Pix22_En_Pin
                          |Pix24_Xs_Pin|Pix24_En_Pin|Pix24_Dir_Pin|Pix24_PW_Pin
                          |Pix07_Dir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix17_Dir_Pin Pix17_PW_Pin Pix18_Xs_Pin Pix18_En_Pin
                           Pix18_Dir_Pin Pix22_Dir_Pin Pix26_En_Pin Pix26_Dir_Pin
                           Pix26_PW_Pin Pix27_Xs_Pin Pix27_En_Pin Pix27_Dir_Pin
                           Pix03_Dir_Pin Pix03_En_Pin Pix03_Xs_Pin Pix04_PW_Pin */
  GPIO_InitStruct.Pin = Pix17_Dir_Pin|Pix17_PW_Pin|Pix18_Xs_Pin|Pix18_En_Pin
                          |Pix18_Dir_Pin|Pix22_Dir_Pin|Pix26_En_Pin|Pix26_Dir_Pin
                          |Pix26_PW_Pin|Pix27_Xs_Pin|Pix27_En_Pin|Pix27_Dir_Pin
                          |Pix03_Dir_Pin|Pix03_En_Pin|Pix03_Xs_Pin|Pix04_PW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix19_Dir_Pin Pix19_PW_Pin Pix28_Dir_Pin Pix28_PW_Pin
                           Pix29_Xs_Pin Pix29_En_Pin Pix29_Dir_Pin Pix29_PW_Pin
                           Pix30_Xs_Pin Pix04_Dir_Pin Pix04_En_Pin Pix04_Xs_Pin
                           Pix05_PW_Pin Pix05_Dir_Pin Pix05_En_Pin Pix07_PW_Pin */
  GPIO_InitStruct.Pin = Pix19_Dir_Pin|Pix19_PW_Pin|Pix28_Dir_Pin|Pix28_PW_Pin
                          |Pix29_Xs_Pin|Pix29_En_Pin|Pix29_Dir_Pin|Pix29_PW_Pin
                          |Pix30_Xs_Pin|Pix04_Dir_Pin|Pix04_En_Pin|Pix04_Xs_Pin
                          |Pix05_PW_Pin|Pix05_Dir_Pin|Pix05_En_Pin|Pix07_PW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix24_XsD10_Pin Pix25_En_Pin Pix25_Dir_Pin Pix25_PW_Pin
                           Pix26_Xs_Pin Pix01_Xs_Pin Pix02_PW_Pin Pix02_Dir_Pin
                           Pix02_En_Pin Pix02_Xs_Pin Pix03_PW_Pin */
  GPIO_InitStruct.Pin = Pix24_XsD10_Pin|Pix25_En_Pin|Pix25_Dir_Pin|Pix25_PW_Pin
                          |Pix26_Xs_Pin|Pix01_Xs_Pin|Pix02_PW_Pin|Pix02_Dir_Pin
                          |Pix02_En_Pin|Pix02_Xs_Pin|Pix03_PW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : Pix27_PW_Pin Pix28_Xs_Pin Pix28_En_Pin Pix05_Xs_Pin
                           Pix06_PW_Pin Pix06_Dir_Pin Pix06_En_Pin Pix06_Xs_Pin */
  GPIO_InitStruct.Pin = Pix27_PW_Pin|Pix28_Xs_Pin|Pix28_En_Pin|Pix05_Xs_Pin
                          |Pix06_PW_Pin|Pix06_Dir_Pin|Pix06_En_Pin|Pix06_Xs_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
