/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_sd.h"
#include "stm32l4r9i_discovery.h"
#include "stm32l4r9i_discovery_io.h"
#include "stm32l4r9i_discovery_sd.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

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
#define OCTOSPIM_P2_IO1_Pin GPIO_PIN_10
#define OCTOSPIM_P2_IO1_GPIO_Port GPIOI
#define MIC_VDD_Pin GPIO_PIN_2
#define MIC_VDD_GPIO_Port GPIOH
#define PSRAM_NBL0_Pin GPIO_PIN_0
#define PSRAM_NBL0_GPIO_Port GPIOE
#define ARD_D6_Pin GPIO_PIN_4
#define ARD_D6_GPIO_Port GPIOB
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define USART3_RTS_Pin GPIO_PIN_15
#define USART3_RTS_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define ARD_10_Pin GPIO_PIN_0
#define ARD_10_GPIO_Port GPIOI
#define DSI_SPI_USART_CS_Pin GPIO_PIN_14
#define DSI_SPI_USART_CS_GPIO_Port GPIOH
#define OCTOSPIM_P2_IO2_Pin GPIO_PIN_9
#define OCTOSPIM_P2_IO2_GPIO_Port GPIOI
#define DCMI_D7_Pin GPIO_PIN_7
#define DCMI_D7_GPIO_Port GPIOI
#define PSRAM_NBL1_Pin GPIO_PIN_1
#define PSRAM_NBL1_GPIO_Port GPIOE
#define SAI1_SDB_Pin GPIO_PIN_5
#define SAI1_SDB_GPIO_Port GPIOB
#define OCTOSPIM_P2_IO6_Pin GPIO_PIN_9
#define OCTOSPIM_P2_IO6_GPIO_Port GPIOG
#define D2_Pin GPIO_PIN_0
#define D2_GPIO_Port GPIOD
#define OCTOSPIM_P2_CLK_Pin GPIO_PIN_6
#define OCTOSPIM_P2_CLK_GPIO_Port GPIOI
#define MFX_IRQ_OUT_Pin GPIO_PIN_1
#define MFX_IRQ_OUT_GPIO_Port GPIOI
#define DCMI_D3_Pin GPIO_PIN_12
#define DCMI_D3_GPIO_Port GPIOH
#define OCTOSPIM_P2_IO0_Pin GPIO_PIN_11
#define OCTOSPIM_P2_IO0_GPIO_Port GPIOI
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define OCTOSPIM_P2_DQS_Pin GPIO_PIN_15
#define OCTOSPIM_P2_DQS_GPIO_Port GPIOG
#define PSRAM_OE_Pin GPIO_PIN_4
#define PSRAM_OE_GPIO_Port GPIOD
#define D3_Pin GPIO_PIN_1
#define D3_GPIO_Port GPIOD
#define ARD_D9_Pin GPIO_PIN_13
#define ARD_D9_GPIO_Port GPIOH
#define OCTOPSIM_P2_IO4_Pin GPIO_PIN_9
#define OCTOPSIM_P2_IO4_GPIO_Port GPIOH
#define PSRAM_A20_Pin GPIO_PIN_4
#define PSRAM_A20_GPIO_Port GPIOE
#define PSRAM_A19_Pin GPIO_PIN_3
#define PSRAM_A19_GPIO_Port GPIOE
#define SAI1_MCKA_Pin GPIO_PIN_2
#define SAI1_MCKA_GPIO_Port GPIOE
#define SAI1_FSA_Pin GPIO_PIN_9
#define SAI1_FSA_GPIO_Port GPIOB
#define PSRAM_ADV_Pin GPIO_PIN_7
#define PSRAM_ADV_GPIO_Port GPIOB
#define OCTOSPIM_P2_IO7_Pin GPIO_PIN_10
#define OCTOSPIM_P2_IO7_GPIO_Port GPIOG
#define PSRAM_WE_Pin GPIO_PIN_5
#define PSRAM_WE_GPIO_Port GPIOD
#define uSD_CMD_Pin GPIO_PIN_2
#define uSD_CMD_GPIO_Port GPIOD
#define uSD_D2_Pin GPIO_PIN_10
#define uSD_D2_GPIO_Port GPIOC
#define DCMI_D5_Pin GPIO_PIN_4
#define DCMI_D5_GPIO_Port GPIOI
#define USB_OTGFS_ID_Pin GPIO_PIN_10
#define USB_OTGFS_ID_GPIO_Port GPIOA
#define USB_OTG_FS_DP_Pin GPIO_PIN_12
#define USB_OTG_FS_DP_GPIO_Port GPIOA
#define JOY_SEL_Pin GPIO_PIN_13
#define JOY_SEL_GPIO_Port GPIOC
#define SAI1_SDA_Pin GPIO_PIN_6
#define SAI1_SDA_GPIO_Port GPIOE
#define SAI1_SCKA_Pin GPIO_PIN_5
#define SAI1_SCKA_GPIO_Port GPIOE
#define PSRAM_WAIT_Pin GPIO_PIN_6
#define PSRAM_WAIT_GPIO_Port GPIOD
#define PSRAM_CLK_Pin GPIO_PIN_3
#define PSRAM_CLK_GPIO_Port GPIOD
#define uSD_D3_Pin GPIO_PIN_11
#define uSD_D3_GPIO_Port GPIOC
#define DCMI_VSYNC_Pin GPIO_PIN_5
#define DCMI_VSYNC_GPIO_Port GPIOI
#define USB_OTGFS_VBUS_Pin GPIO_PIN_9
#define USB_OTGFS_VBUS_GPIO_Port GPIOA
#define USB_OTGFS_DM_Pin GPIO_PIN_11
#define USB_OTGFS_DM_GPIO_Port GPIOA
#define PSRAM_A2_Pin GPIO_PIN_2
#define PSRAM_A2_GPIO_Port GPIOF
#define PSRAM_A1_Pin GPIO_PIN_1
#define PSRAM_A1_GPIO_Port GPIOF
#define PSRAM_A0_Pin GPIO_PIN_0
#define PSRAM_A0_GPIO_Port GPIOF
#define OCTOSPIM_P2_CS_Pin GPIO_PIN_12
#define OCTOSPIM_P2_CS_GPIO_Port GPIOG
#define PSRAM_NE1_Pin GPIO_PIN_7
#define PSRAM_NE1_GPIO_Port GPIOD
#define uSD_CLK_Pin GPIO_PIN_12
#define uSD_CLK_GPIO_Port GPIOC
#define uSD_D0_Pin GPIO_PIN_8
#define uSD_D0_GPIO_Port GPIOC
#define ARD_14_Pin GPIO_PIN_8
#define ARD_14_GPIO_Port GPIOG
#define STMOD_INT_Pin GPIO_PIN_6
#define STMOD_INT_GPIO_Port GPIOC
#define PSRAM_A3_Pin GPIO_PIN_3
#define PSRAM_A3_GPIO_Port GPIOF
#define PSRAM_A4_Pin GPIO_PIN_4
#define PSRAM_A4_GPIO_Port GPIOF
#define PSRAM_A5_Pin GPIO_PIN_5
#define PSRAM_A5_GPIO_Port GPIOF
#define I2C1_SDA_Pin GPIO_PIN_13
#define I2C1_SDA_GPIO_Port GPIOG
#define PSRAM_A14_Pin GPIO_PIN_4
#define PSRAM_A14_GPIO_Port GPIOG
#define PSRAM_A13_Pin GPIO_PIN_3
#define PSRAM_A13_GPIO_Port GPIOG
#define PSRAM_A15_Pin GPIO_PIN_5
#define PSRAM_A15_GPIO_Port GPIOG
#define ARD_15_Pin GPIO_PIN_7
#define ARD_15_GPIO_Port GPIOG
#define DFDATIN3_Pin GPIO_PIN_7
#define DFDATIN3_GPIO_Port GPIOC
#define uSD_D1_Pin GPIO_PIN_9
#define uSD_D1_GPIO_Port GPIOC
#define PSRAM_A11_Pin GPIO_PIN_1
#define PSRAM_A11_GPIO_Port GPIOG
#define D7_Pin GPIO_PIN_10
#define D7_GPIO_Port GPIOE
#define USART3_RX_Pin GPIO_PIN_11
#define USART3_RX_GPIO_Port GPIOB
#define PSRAM_A18_Pin GPIO_PIN_13
#define PSRAM_A18_GPIO_Port GPIOD
#define PSRAM_A12_Pin GPIO_PIN_2
#define PSRAM_A12_GPIO_Port GPIOG
#define D1_Pin GPIO_PIN_15
#define D1_GPIO_Port GPIOD
#define D0_Pin GPIO_PIN_14
#define D0_GPIO_Port GPIOD
#define ARD_D0_Pin GPIO_PIN_0
#define ARD_D0_GPIO_Port GPIOC
#define ARD_D1_Pin GPIO_PIN_1
#define ARD_D1_GPIO_Port GPIOC
#define DF_CKOUT_Pin GPIO_PIN_2
#define DF_CKOUT_GPIO_Port GPIOC
#define PSRAM_A10_Pin GPIO_PIN_0
#define PSRAM_A10_GPIO_Port GPIOG
#define D6_Pin GPIO_PIN_9
#define D6_GPIO_Port GPIOE
#define D12_Pin GPIO_PIN_15
#define D12_GPIO_Port GPIOE
#define PSRAM_A17_Pin GPIO_PIN_12
#define PSRAM_A17_GPIO_Port GPIOD
#define PSRAM_A16_Pin GPIO_PIN_11
#define PSRAM_A16_GPIO_Port GPIOD
#define D15_Pin GPIO_PIN_10
#define D15_GPIO_Port GPIOD
#define ARD_A2_Pin GPIO_PIN_3
#define ARD_A2_GPIO_Port GPIOC
#define ARD_A4_Pin GPIO_PIN_0
#define ARD_A4_GPIO_Port GPIOA
#define ARD_A1_Pin GPIO_PIN_4
#define ARD_A1_GPIO_Port GPIOC
#define PSRAM_A9_Pin GPIO_PIN_15
#define PSRAM_A9_GPIO_Port GPIOF
#define D5_Pin GPIO_PIN_8
#define D5_GPIO_Port GPIOE
#define D11_Pin GPIO_PIN_14
#define D11_GPIO_Port GPIOE
#define LED_GREEN_Pin GPIO_PIN_4
#define LED_GREEN_GPIO_Port GPIOH
#define D14_Pin GPIO_PIN_9
#define D14_GPIO_Port GPIOD
#define D13_Pin GPIO_PIN_8
#define D13_GPIO_Port GPIOD
#define STMOD_PWM_Pin GPIO_PIN_5
#define STMOD_PWM_GPIO_Port GPIOA
#define SPI2_CS_Pin GPIO_PIN_6
#define SPI2_CS_GPIO_Port GPIOA
#define PSRAM_A8_Pin GPIO_PIN_14
#define PSRAM_A8_GPIO_Port GPIOF
#define D4_Pin GPIO_PIN_7
#define D4_GPIO_Port GPIOE
#define D10_Pin GPIO_PIN_13
#define D10_GPIO_Port GPIOE
#define DCMI_PIXCLK_Pin GPIO_PIN_5
#define DCMI_PIXCLK_GPIO_Port GPIOH
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOB
#define ARD_D5_Pin GPIO_PIN_1
#define ARD_D5_GPIO_Port GPIOA
#define USART2_RX_Pin GPIO_PIN_3
#define USART2_RX_GPIO_Port GPIOA
#define ARD_A0_Pin GPIO_PIN_7
#define ARD_A0_GPIO_Port GPIOA
#define DSI_TE_Pin GPIO_PIN_11
#define DSI_TE_GPIO_Port GPIOF
#define PSRAM_A7_Pin GPIO_PIN_13
#define PSRAM_A7_GPIO_Port GPIOF
#define D9_Pin GPIO_PIN_12
#define D9_GPIO_Port GPIOE
#define OCTOSPIM_P2_IO5_Pin GPIO_PIN_10
#define OCTOSPIM_P2_IO5_GPIO_Port GPIOH
#define DCMI_D2_Pin GPIO_PIN_11
#define DCMI_D2_GPIO_Port GPIOH
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MISO_GPIO_Port GPIOB
#define USART2_TX_Pin GPIO_PIN_2
#define USART2_TX_GPIO_Port GPIOA
#define DCMI_HSYNC_Pin GPIO_PIN_4
#define DCMI_HSYNC_GPIO_Port GPIOA
#define ARD_A3_Pin GPIO_PIN_0
#define ARD_A3_GPIO_Port GPIOB
#define MFX_WAKEUP_Pin GPIO_PIN_2
#define MFX_WAKEUP_GPIO_Port GPIOB
#define PSRAM_A6_Pin GPIO_PIN_12
#define PSRAM_A6_GPIO_Port GPIOF
#define D8_Pin GPIO_PIN_11
#define D8_GPIO_Port GPIOE
#define USART3_TX_Pin GPIO_PIN_10
#define USART3_TX_GPIO_Port GPIOB
#define OCTOSPI_P2_IO3_Pin GPIO_PIN_8
#define OCTOSPI_P2_IO3_GPIO_Port GPIOH
#define DFDATIN1_Pin GPIO_PIN_12
#define DFDATIN1_GPIO_Port GPIOB
#define SPI2_CLK_Pin GPIO_PIN_13
#define SPI2_CLK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
