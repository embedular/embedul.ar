/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define S800_UART4_TX_Pin GPIO_PIN_0
#define S800_UART4_TX_GPIO_Port GPIOA
#define S800_UART4_RX_Pin GPIO_PIN_1
#define S800_UART4_RX_GPIO_Port GPIOA
#define S800_DTR_Pin GPIO_PIN_2
#define S800_DTR_GPIO_Port GPIOA
#define S800_ON_Pin GPIO_PIN_3
#define S800_ON_GPIO_Port GPIOA
#define EXP_SPI1_NSS_Pin GPIO_PIN_4
#define EXP_SPI1_NSS_GPIO_Port GPIOA
#define EXP_SPI1_SCK_Pin GPIO_PIN_5
#define EXP_SPI1_SCK_GPIO_Port GPIOA
#define EXP_SPI1_MISO_Pin GPIO_PIN_6
#define EXP_SPI1_MISO_GPIO_Port GPIOA
#define EXP_SPI1_MOSI_Pin GPIO_PIN_7
#define EXP_SPI1_MOSI_GPIO_Port GPIOA
#define EXP_PIN1_Pin GPIO_PIN_4
#define EXP_PIN1_GPIO_Port GPIOC
#define EXP_PIN3_Pin GPIO_PIN_5
#define EXP_PIN3_GPIO_Port GPIOC
#define EXP_PIN5_Pin GPIO_PIN_0
#define EXP_PIN5_GPIO_Port GPIOB
#define EXP_PIN7_Pin GPIO_PIN_1
#define EXP_PIN7_GPIO_Port GPIOB
#define EXP_PIN9_Pin GPIO_PIN_7
#define EXP_PIN9_GPIO_Port GPIOE
#define EXP_PIN11_Pin GPIO_PIN_8
#define EXP_PIN11_GPIO_Port GPIOE
#define EXP_PIN10_Pin GPIO_PIN_9
#define EXP_PIN10_GPIO_Port GPIOE
#define EXP_PIN8_Pin GPIO_PIN_10
#define EXP_PIN8_GPIO_Port GPIOE
#define EXP_PIN6_Pin GPIO_PIN_11
#define EXP_PIN6_GPIO_Port GPIOE
#define EXP_PIN4_Pin GPIO_PIN_12
#define EXP_PIN4_GPIO_Port GPIOE
#define EXP_PIN2_Pin GPIO_PIN_13
#define EXP_PIN2_GPIO_Port GPIOE
#define RS845_RE_Pin GPIO_PIN_14
#define RS845_RE_GPIO_Port GPIOE
#define RS485_DE_Pin GPIO_PIN_15
#define RS485_DE_GPIO_Port GPIOE
#define RS485_USART3_TX_Pin GPIO_PIN_10
#define RS485_USART3_TX_GPIO_Port GPIOB
#define RS485_USART3_RX_Pin GPIO_PIN_11
#define RS485_USART3_RX_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_15
#define LED4_GPIO_Port GPIOB
#define IN_READ_Pin GPIO_PIN_8
#define IN_READ_GPIO_Port GPIOD
#define OUT6_Pin GPIO_PIN_9
#define OUT6_GPIO_Port GPIOD
#define OUT5_Pin GPIO_PIN_10
#define OUT5_GPIO_Port GPIOD
#define OUT4_Pin GPIO_PIN_11
#define OUT4_GPIO_Port GPIOD
#define OUT3_Pin GPIO_PIN_12
#define OUT3_GPIO_Port GPIOD
#define OUT2_Pin GPIO_PIN_13
#define OUT2_GPIO_Port GPIOD
#define OUT1_Pin GPIO_PIN_14
#define OUT1_GPIO_Port GPIOD
#define IN_CLK_Pin GPIO_PIN_15
#define IN_CLK_GPIO_Port GPIOD
#define EXP_USART6_TX_Pin GPIO_PIN_6
#define EXP_USART6_TX_GPIO_Port GPIOC
#define EXP_USART6_RX_Pin GPIO_PIN_7
#define EXP_USART6_RX_GPIO_Port GPIOC
#define IN_PS_Pin GPIO_PIN_9
#define IN_PS_GPIO_Port GPIOC
#define OUT_EN_Pin GPIO_PIN_8
#define OUT_EN_GPIO_Port GPIOA
#define ESP_USART1_TX_Pin GPIO_PIN_9
#define ESP_USART1_TX_GPIO_Port GPIOA
#define ESP_USART1_RX_Pin GPIO_PIN_10
#define ESP_USART1_RX_GPIO_Port GPIOA
#define ESP_RESET_Pin GPIO_PIN_11
#define ESP_RESET_GPIO_Port GPIOA
#define SD_DET_Pin GPIO_PIN_12
#define SD_DET_GPIO_Port GPIOA
#define SW1_Pin GPIO_PIN_10
#define SW1_GPIO_Port GPIOC
#define SW2_Pin GPIO_PIN_11
#define SW2_GPIO_Port GPIOC
#define SD_CD_Pin GPIO_PIN_0
#define SD_CD_GPIO_Port GPIOD
#define A6_INT_Pin GPIO_PIN_3
#define A6_INT_GPIO_Port GPIOD
#define A6_ON_Pin GPIO_PIN_4
#define A6_ON_GPIO_Port GPIOD
#define A6_USART2_TX_Pin GPIO_PIN_5
#define A6_USART2_TX_GPIO_Port GPIOD
#define A6_USART2_RX_Pin GPIO_PIN_6
#define A6_USART2_RX_GPIO_Port GPIOD
#define EXP_I2C1_SCL_Pin GPIO_PIN_6
#define EXP_I2C1_SCL_GPIO_Port GPIOB
#define EXP_I2C1_SDA_Pin GPIO_PIN_7
#define EXP_I2C1_SDA_GPIO_Port GPIOB
#define EXP_CAN1_RX_Pin GPIO_PIN_8
#define EXP_CAN1_RX_GPIO_Port GPIOB
#define EXP_CAN1_TX_Pin GPIO_PIN_9
#define EXP_CAN1_TX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
