/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "supercap.h"
#include <stdio.h>
#include "Serial.h"
#include "Kalman_Filter.h"

#define MAX_CAP_VOLTAGE 2700 //24V in 12 bits
#define MAX_CURRENT 2202 //1A in 12 bits
#define MIN_CURRENT 2047 //0A in 12 bits
#define MAX_DUTY 278
#define MIN_DUTY 22
#define V_1V65 2047 //1.65V in 12 bits

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
  uint16_t supercap_ADC1[2];
  uint16_t supercap_ADC2[3];
  uint16_t supercap_ADC3[3];
	uint16_t supercap_ADC4[2];

  _ADC_Sample_t C_left = {0};
  _ADC_Sample_t C_sys = {0};
  _ADC_Sample_t C_right = {0};
  _ADC_Sample_t V_sys_op = {0};
  _ADC_Sample_t V_cap_op = {0};
  _ADC_Sample_t V_cap = {0}; //V_right
  _ADC_Sample_t V_bat = {0}; //V_left
  _ADC_Sample_t V_sys = {0};
	
	_ADC_Sample_t V_1V6 = {0};
	_ADC_Sample_t ADC4_12 = {0};

	Kalman_Filter_t C_left_kalman = {.Q=0.5f,.R=1.0f};
	
  _Supercap_PID_Controller_t PID_45W_loop;
  _Supercap_PID_Controller_t PID_24V_loop;
  _Supercap_PID_Controller_t PID_7A_loop;
	_Supercap_PID_Controller_t PID_voltage_loop;
	
	float temp2;
	uint16_t temp_counter;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
//{
//  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
//}
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
  
  char tail[4] = {0x00, 0x00, 0x80, 0x7f};

  C_left.sample = &supercap_ADC1[0];
  C_sys.sample = &supercap_ADC1[1];
  C_right.sample = &supercap_ADC3[2];
  V_sys_op.sample = &supercap_ADC2[0];
  V_cap_op.sample = &supercap_ADC2[1];
  V_cap.sample = &supercap_ADC2[2];
  V_bat.sample = &supercap_ADC3[0];
  V_sys.sample = &supercap_ADC3[1];
	V_1V6.sample = &supercap_ADC4[0];
	ADC4_12.sample = &supercap_ADC4[1];
	
	float temp;
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_ADC4_Init();
  MX_FDCAN1_Init();
  MX_ADC5_Init();
  MX_ADC3_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)supercap_ADC1, 2);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)supercap_ADC2, 3);
  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)supercap_ADC3, 3);
	HAL_ADC_Start_DMA(&hadc4, (uint32_t*)supercap_ADC4, 2);
	
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_4);
	
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	
	//Supercap_Soft_Start();
	
	Supercap_Function_Init(&C_sys);
	Supercap_Function_Init(&C_right);
	Supercap_Function_Init(&ADC4_12);
	Supercap_Function_Init(&V_bat);
	Supercap_Function_Init(&V_cap);

  Supercap_PID_Init(&PID_45W_loop, 0.001f, 0.0001f, 0.0f, MAX_CAP_VOLTAGE, 0, 2500);
  Supercap_PID_Init(&PID_24V_loop, 1.0f, 0.000f, 0.0f, MAX_CAP_VOLTAGE, 0, 2500);
  Supercap_PID_Init(&PID_7A_loop, 1.0f, 0.000f, 0.0f, MAX_CAP_VOLTAGE, 0, 2500);
	Supercap_PID_Init(&PID_voltage_loop, 0.00001f, 0.000001f, 0.05f, MAX_DUTY, 0, 30);
	
  Supercap_PWM_left(0);

HAL_TIM_Base_Start_IT(&htim2);
  //while(V_cap.real_value_12bits<11){};
	HAL_Delay(1000);
	//HAL_TIM_Base_Start_IT(&htim5);

	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //sending set: One side DCDC test bench
    //Current, voltage and duty cycle
    /*
		temp = Supercap_ADC_to_Current_Funtion(C_sys.real_value_12bits,2047);
		HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = PID_45W_loop.output;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = Supercap_ADC_to_Voltage_Funtion(V_cap.real_value_12bits);
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = PID_7A_loop.output;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = temp2;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
		temp = PID_voltage_loop.output;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);*/

    //test set 2: This set test the limitation of C_right
		temp = Supercap_ADC_to_Current_Funtion(C_sys.real_value_12bits,2047);
		HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
		temp = Supercap_ADC_to_Current_Funtion(C_right.real_value_12bits,2047);
		HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = Supercap_ADC_to_Voltage_Funtion(V_cap.real_value_12bits);
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = Supercap_ADC_to_Voltage_Funtion(PID_7A_loop.output);
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = Supercap_ADC_to_Voltage_Funtion(PID_45W_loop.output);
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = Supercap_ADC_to_Voltage_Funtion(temp2); //output PWM
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
    temp = temp2 * 0.1f;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);
		temp = PID_voltage_loop.output;
    HAL_UART_Transmit(&huart3, (uint8_t*)&(temp), 4, 1000);

		HAL_UART_Transmit(&huart3, (uint8_t*)tail, 4, 1000);
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 75;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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