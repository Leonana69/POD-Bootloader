/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "boot.h"
#include "config.h"
#include "crtp.h"
#include "syslink.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  bootpinInit();
  // if (bootpinStartFirmware() == true) {
  //   if (*((uint32_t*)FIRMWARE_START) != 0xFFFFFFFFU) {
  //     void (*firmware)(void) __attribute__((noreturn)) = (void *)(*(uint32_t*)(FIRMWARE_START + 4));
  //     bootpinDeinit();
  //     // Start firmware
  //     /* HAL implementation */
  //     SCB->VTOR = FIRMWARE_START | 0;
  //     /* STD library implementation */
  //     // NVIC_SetVectorTable(FIRMWARE_START, 0);
  //     __set_MSP(*((uint32_t*) FIRMWARE_START));
  //     firmware();
  //   }
  // } else 
  if (bootpinNrfReset() == true) {
    void (*bootloader)(void) __attribute__((noreturn)) = (void *)(*(uint32_t*)(SYSTEM_BASE + 4));
    bootpinDeinit();
    // Start bootloader
    SCB->VTOR = SYSTEM_BASE | 0;
    __set_MSP(*((uint32_t*) SYSTEM_BASE));
    bootloader();
  }
  bootpinDeinit();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_SetTickFreq(HAL_TICK_FREQ_1KHZ);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  unsigned int ledGreenTime = 0;
  unsigned int ledRedTime = 0;
  unsigned int ledBlueTime = 0;
  CrtpPacket packet;
  struct syslinkPacket slPacket;
  HAL_GPIO_WritePin(BLUE_L_GPIO_Port, BLUE_L_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
  while (1) {
    if (syslinkReceive(&slPacket)) {
      if (slPacket.type == SYSLINK_RADIO_RAW) {
        memcpy(packet.raw, slPacket.data, slPacket.length);
        packet.datalen = slPacket.length-1;

        ledGreenTime = HAL_GetTick();
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);

        if (bootloaderProcess(&packet)) {
          ledRedTime = HAL_GetTick();
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);

          memcpy(slPacket.data, packet.raw, packet.datalen + 1);
          slPacket.length = packet.datalen + 1;
          syslinkSend(&slPacket);
        }
      }
    }

    if (ledGreenTime != 0 && HAL_GetTick() - ledGreenTime > 10) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
      ledGreenTime = 0;
    }
    if (ledRedTime != 0 && HAL_GetTick() - ledRedTime > 10) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
      ledRedTime = 0;
    }

    if ((HAL_GetTick() - ledBlueTime) > 500) {
      if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_SET) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 0);
      } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, 1);
      }
      ledBlueTime = HAL_GetTick();
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
