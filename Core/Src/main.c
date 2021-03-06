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
#include "crc.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmp280_config.h"
#include "lcd_config.h"
#include "encoder_config.h"
#include "encoder.h"
#include "heater_pwm_config.h"
#include "heater_pwm.h"
#include "fan_pwm.h"
#include "fan_pwm_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
char rx_buffer[4];
uint16_t new_value;
uint8_t fan_percent=0;
uint8_t heater_percent=0;
uint16_t start_value = 2500;



/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 {
	if(huart->Instance==USART3)
	 {
		int i;
		sscanf(rx_buffer, "%d", &i);
		if (i<=3500 && i>=2500)	new_value = i;
		ENC_SetCounter(&henc1,4*(new_value - start_value)/10);

	 }
	HAL_UART_Receive_IT(&huart3, (uint8_t*)rx_buffer, 4);
 }

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



uint32_t crcVal;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	struct bmp280_uncomp_data bmp280_1_data;
	int32_t temp32;
	double temp;
	char message[20];
	uint32_t encoder_count; //Encoder value
	float error=0;
	float reset=0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();
  MX_SPI4_Init();
  MX_TIM5_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  /** Sensor initialization *******************************************/
  BMP280_Init(&bmp280_1);

  /** Heater PWM initialization *******************************************/

  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  //__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  HEATER_PWM_Init(&heaterpwm1);
  HEATER_PWM_SetDuty(&heaterpwm1, heater_percent);

  /** Fan PWM initialization *******************************************/
  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  //__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  FAN_PWM_Init(&fanpwm1);
  FAN_PWM_SetDuty(&fanpwm1, fan_percent);

  HAL_UART_Receive_IT(&huart3, (uint8_t*)rx_buffer, 4);

  /** LCD with user menu initialization **************************************************/
   LCD_Init(&hlcd1);

  /** Rotary quadrature encoder initialization *******************************************/
   ENC_Init(&henc1);


   //encoder_count = ENC_GetCounter(&henc1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  // Read rotary encoder counter

	  encoder_count = ENC_GetCounter(&henc1);
	  new_value = (int)(start_value + 10 * ((float)encoder_count)/4.0);



	  /* Reading the raw data from sensor */
	  bmp280_get_uncomp_data(&bmp280_1_data, &bmp280_1);

	  /* Getting the 32 bit compensated temperature */
	  bmp280_get_comp_temp_32bit(&temp32, bmp280_1_data.uncomp_temp, &bmp280_1);


	  // temp destination, temp actual, fan speed percentage
	  _LCD_Show(&hlcd1, new_value,temp32 ,fan_percent);

	  // char messagetemp destination, temp actual, fan speed percentage
	  _Message_Generate(&message,temp32, new_value, fan_percent);

	  HAL_UART_Transmit(&huart3, (uint8_t*)message,  strlen(message), 1000);

	  HAL_Delay(100);

	  error = (new_value - temp32)/1000.0;

	  if(error>0) // heater(reistor)
	  {
		  heater_percent = 100;
		  fan_percent = 0;
		  HEATER_PWM_SetDuty(&heaterpwm1, heater_percent);
		  FAN_PWM_SetDuty(&fanpwm1, fan_percent);
	  }
	  else if(error<0) //cooler(fan)
	  {
		  fan_percent = 100;
		  heater_percent = 0;
		  HEATER_PWM_SetDuty(&heaterpwm1, heater_percent);
		  FAN_PWM_SetDuty(&fanpwm1, fan_percent);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_I2C1;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
