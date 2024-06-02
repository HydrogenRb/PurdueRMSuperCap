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