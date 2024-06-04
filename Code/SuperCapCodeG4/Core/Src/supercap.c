/**
  ******************************************************************************
  * @file    supercap.c
  * @brief   This file provides code for the configuration
  *          of all used supercap pins.
  */
/* USER CODE END Header */
#include "supercap.h"

#define SAMPLE_SIZE 4



float ADC_to_Voltage_Funtion(uint16_t adc_value) {
  float voltage = (float)adc_value * 3.3 / 4095.0;
  return voltage;
}

void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample) {
    if(adc_sample->count == 0){
      adc_sample->average = *(adc_sample->sample);
      //adc_sample->history_sum = adc_sample->average * SAMPLE_SIZE;
      adc_sample->history_sum = adc_sample->average << 2;
      adc_sample->count = 1;
    }
    adc_sample->history_sum = adc_sample->history_sum - adc_sample->average + *(adc_sample->sample);
    //adc_sample->average = adc_sample->history_sum / SAMPLE_SIZE;
    adc_sample->average = adc_sample->history_sum >> 2; //想看看这个地方能不能快一点
    adc_sample->real_value = ADC_to_Voltage_Funtion(adc_sample->average);
}

void Supercap_PID_Controller_Function(_Supercap_PID_Controller_t *pid_controller, _ADC_Sample_t *adc_sample, float target) {
  pid_controller->error = target - adc_sample->real_value;
  pid_controller->error_sum += pid_controller->error;
  pid_controller->output = pid_controller->Kp * pid_controller->error + pid_controller->Ki * pid_controller->error_sum + pid_controller->Kd * (pid_controller->error - pid_controller->error_last);
  pid_controller->error_last = pid_controller->error;
  if(pid_controller->output > pid_controller->output_max) {
    pid_controller->output = pid_controller->output_max;
  }
  if(pid_controller->output < pid_controller->output_min) {
    pid_controller->output = pid_controller->output_min;
  }
}

//帮我在这加入tim2的中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim->Instance == TIM2) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    TIM2_NVIC();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  }
}

void TIM2_NVIC(){
    Supercap_Average_ADC_Function(&C_left);
    Supercap_Average_ADC_Function(&C_sys);
    Supercap_Average_ADC_Function(&C_right);
    Supercap_Average_ADC_Function(&V_sys_op);
    Supercap_Average_ADC_Function(&V_cap_op);
    Supercap_Average_ADC_Function(&V_cap);
    Supercap_Average_ADC_Function(&V_bat);
    Supercap_Average_ADC_Function(&V_sys);
		
		Supercap_Average_ADC_Function(&V_1V6);

}