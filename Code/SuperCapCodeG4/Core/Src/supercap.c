/**
  ******************************************************************************
  * @file    supercap.c
  * @brief   This file provides code for the configuration
  *          of all used supercap pins.
  */
/* USER CODE END Header */
#include "supercap.h"




float ADC_to_Voltage_Funtion(uint16_t adc_value) {
  float voltage = (float)adc_value * 3.3f / 4095.0f;
  return voltage;
}

void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample) {
			adc_sample->real_value_12bits = (*(adc_sample->sample)>>3) + (adc_sample->real_value_12bits - (adc_sample->real_value_12bits>>3));
}

void Supercap_PID_Controller_Function(_Supercap_PID_Controller_t *pid_controller, _ADC_Sample_t *adc_sample, int16_t target) {
	pid_controller->error = target - adc_sample->real_value_12bits;
  pid_controller->error_sum += pid_controller->error * pid_controller->Ki;
  
  if(pid_controller->error_sum > pid_controller->I_max) {
    pid_controller->error_sum = pid_controller->I_max;
  }else if(pid_controller->error_sum < -pid_controller->I_max) {
    pid_controller->error_sum = -pid_controller->I_max;
  }

  pid_controller->output = pid_controller->Kp * pid_controller->error + pid_controller->error_sum + pid_controller->Kd * (pid_controller->error - pid_controller->error_last);
  
  if(pid_controller->output > pid_controller->output_max) {
    pid_controller->output = pid_controller->output_max;
  }
  else if(pid_controller->output < pid_controller->output_min) {
    pid_controller->output = pid_controller->output_min;
  }
	
	pid_controller->error_last = pid_controller->error;
}

void Supercap_PID_Init(_Supercap_PID_Controller_t *pid_controller, float Kp, float Ki, float Kd, int16_t output_max, int16_t output_min, int16_t I_max) {
  pid_controller->Kp = Kp;
  pid_controller->Ki = Ki;
  pid_controller->Kd = Kd;
  pid_controller->output_max = output_max;
  pid_controller->output_min = output_min;
  pid_controller->I_max = I_max;
}

void Supercap_PWM_left(uint32_t pwm_duty_cycle) {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_duty_cycle);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1);
};


//帮我在这加入tim2的中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIM5){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    TIM5_NVIC();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
	}
  else if(htim->Instance == TIM2) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    TIM2_NVIC();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
  }
}

void TIM2_NVIC(){
    //Supercap_Average_ADC_Function(&C_left);
		Supercap_Average_ADC_Function(&V_1V6);
		temp2 = ADC_to_Voltage_Funtion(*(C_left.sample))-ADC_to_Voltage_Funtion((V_1V6.real_value_12bits));
	  temp2 *= 2;
		C_left.real_value_float = First_Order_Kalman_Filter(&C_left_kalman, temp2);
	  //C_left.real_value_12bits = (int16_t)((C_left.real_value_float * (4095.0f / 3.3f)) + V_1V6.real_value_12bits);
	Supercap_Average_ADC_Function(&C_left);
    Supercap_Average_ADC_Function(&C_sys);
    Supercap_Average_ADC_Function(&C_right);
//    Supercap_Average_ADC_Function(&V_sys_op);
//    Supercap_Average_ADC_Function(&V_cap_op);
    //Supercap_Average_ADC_Function(&V_cap);
    //Supercap_Average_ADC_Function(&V_bat);
    //Supercap_Average_ADC_Function(&V_sys);
		
//		Supercap_Average_ADC_Function(&V_1V6);

}

//This is the control NVIC
void TIM5_NVIC(){
  Supercap_PID_Controller_Function(&Out_loop_PID, &C_left, 2202);
	In_loop_PID.output = 300.0f * ((float)Out_loop_PID.output / 4095.0f);
  //Supercap_PID_Controller_Function(&In_loop_PID, &V_cap, Out_loop_PID.output);
	//Supercap_PID_Controller_Function(&In_loop_PID, &V_cap, 2600);
  //Supercap_PWM_left(In_loop_PID.output);
	Supercap_PWM_left(290);
}


/*
Trash bin


Describe: This filter function is old
void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample) {
    if(adc_sample->count == 0){
      adc_sample->average = *(adc_sample->sample);
      //adc_sample->history_sum = adc_sample->average * SAMPLE_SIZE;
      adc_sample->history_sum = adc_sample->average << 2;
      adc_sample->count = 1;
    }
    adc_sample->history_sum = adc_sample->history_sum - adc_sample->average + *(adc_sample->sample);
    //adc_sample->average = adc_sample->history_sum / SAMPLE_SIZE;
    adc_sample->average = adc_sample->history_sum >> 2;
    adc_sample->real_value = ADC_to_Voltage_Funtion(adc_sample->average);;
}

*/