/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
extern SD_HandleTypeDef hsd_discovery;
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

extern void TIM6_PeriodElapsedCallback();
/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  TIM6_PeriodElapsedCallback();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

void SDMMC1_IRQHandler(void)
{
  HAL_SD_IRQHandler(&hsd_discovery);
}

void LPUART1_IRQHandler(void)
{
  javacall_serial_LPUART_IRQHandler();
}

void USART2_IRQHandler(void)
{
  //HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
  //HAL_UART_IRQHandler(&UARTHandle);
  javacall_serial_USART2_IRQHandler();
}

void EXTI0_IRQHandler(void)
{
  if (IS_GPIO_PIN(JOSH_EXTI0_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI0_PIN);
  }
}

void EXTI1_IRQHandler(void)
{
  javacall_print("HAL_GPIO_EXTI1_Callback\n");
  if (IS_GPIO_PIN(JOSH_EXTI1_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI1_PIN);
  }
}

void EXTI2_IRQHandler(void)
{
  if (IS_GPIO_PIN(JOSH_EXTI2_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI2_PIN);
  }
}

void EXTI3_IRQHandler(void)
{
  if (IS_GPIO_PIN(JOSH_EXTI3_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI3_PIN);
  }
}

void EXTI4_IRQHandler(void)
{ 
  if (IS_GPIO_PIN(JOSH_EXTI4_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI4_PIN);
  }
}

void EXTI9_5_IRQHandler(void)
{
  if (IS_GPIO_PIN(JOSH_EXTI5_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI5_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI6_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI6_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI7_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI7_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI8_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI8_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI9_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI9_PIN);
  }
}

void EXTI15_10_IRQHandler(void)
{
  if (IS_GPIO_PIN(JOSH_EXTI10_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI10_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI11_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI11_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI12_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI12_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI13_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI13_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI14_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI14_PIN);
  }
  if (IS_GPIO_PIN(JOSH_EXTI15_PIN)) {
    HAL_GPIO_EXTI_IRQHandler(JOSH_EXTI15_PIN);
  }
}


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */

/**
  * @}
  */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
