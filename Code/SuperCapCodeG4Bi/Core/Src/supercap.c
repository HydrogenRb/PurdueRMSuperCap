/**
  ******************************************************************************
  * @file    supercap.c
  * @brief   This file provides code for the configuration
  *          of all used supercap pins.
  */
/* USER CODE END Header */
#include "supercap.h"

//Timmer
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

void Supercap_Soft_Start(){
}

//state machine
//state 1(Begin): begin, if V_cap > 0.1V, go to state 2
//state 2(Cap in): stay

void TIM2_NVIC(){
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_sys, 6);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_right, 4);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&ADC4_12,2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_bat, 2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_cap, 4);
	Supercap_First_Order_Filter_ADC_Function_Alpha(&V_sys, 4);
	Supercap_First_Order_Filter_ADC_Function_Alpha(&V_cap_op, 4);
	Supercap_First_Order_Filter_ADC_Function_Alpha(&V_sys_op, 4);
	Supercap_First_Order_Filter_ADC_Function_Alpha(&C_left, 4);
}

void TIM5_NVIC(){
	//Simple test, in and out loop
	Supercap_PID_Controller_Function(&PID_45W_loop, &C_sys, 2360, ((2360.0f-C_sys.real_value_12bits)*5.0f+V_cap.real_value_12bits)); //0 offset, set to +2A
	//Supercap_PID_Controller_Function(&PID_24V_loop, &V_cap, 2707, 0.0f); //0 offset, set to 24V
  Supercap_PID_Controller_Function(&PID_7A_loop, &V_cap, 962, 0.0f); //0 offset, set to +7A or -7A //Change to 4V
	PID_24V_loop.output = 2707;
	//PID_7A_loop.output = 450;
  temp2 = Supercap_Compare(PID_45W_loop.output, PID_24V_loop.output, PID_7A_loop.output);
	Supercap_PID_Controller_Function(&PID_voltage_loop, &V_cap, temp2, 0.07f*temp2);
	Supercap_PWM_left(temp2*0.12f);
}

void Supercap_PID_Controller_Function(_Supercap_PID_Controller_t *pid_controller, _ADC_Sample_t *adc_sample, int16_t target, float offset) {
	pid_controller->error = target - adc_sample->real_value_12bits;
  pid_controller->error_sum += pid_controller->error * pid_controller->Ki;
  if(pid_controller->error_sum > pid_controller->I_max)
  {pid_controller->error_sum = pid_controller->I_max;}
  else if(pid_controller->error_sum < -pid_controller->I_max)
  {pid_controller->error_sum = -pid_controller->I_max;}
  pid_controller->output = pid_controller->Kp*pid_controller->error + pid_controller->error_sum + pid_controller->Kd*(pid_controller->error - pid_controller->error_last) + offset;
  pid_controller->error_last = pid_controller->error;
  if(pid_controller->output > pid_controller->output_max)
  {pid_controller->output = pid_controller->output_max;}
  else if(pid_controller->output < pid_controller->output_min)
  {pid_controller->output = pid_controller->output_min;}
}

int16_t Supercap_Compare(int16_t Output1, int16_t Output2, int16_t Output3){
  if ((Output1 >= Output2 && Output1 <= Output3) || (Output1 <= Output2 && Output1 >= Output3)) {
    return Output1;
  } else if ((Output2 >= Output1 && Output2 <= Output3) || (Output2 <= Output1 && Output2 >= Output3)) {
    return Output2;
  } else {
    return Output3;
  }
}

void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample) {
    if(adc_sample->count == 0){
      adc_sample->average = *(adc_sample->sample);
      adc_sample->history_sum = adc_sample->average << 2;
      adc_sample->count = 1;
    }
    adc_sample->history_sum = adc_sample->history_sum - adc_sample->average + *(adc_sample->sample);
    adc_sample->average = adc_sample->history_sum >> 2;
    adc_sample->real_value_12bits = (adc_sample->average);
}

void Supercap_First_Order_Filter_ADC_Function(_ADC_Sample_t *adc_sample) {
			adc_sample->real_value_12bits = (*(adc_sample->sample)>>3) + (adc_sample->real_value_12bits - (adc_sample->real_value_12bits>>3));
}

void Supercap_First_Order_Filter_ADC_Function_Alpha(_ADC_Sample_t *adc_sample, uint16_t alpha) {
			adc_sample->real_value_12bits = (*(adc_sample->sample)>>alpha) + (adc_sample->real_value_12bits - (adc_sample->real_value_12bits>>alpha));
}

void Supercap_Function_Init(_ADC_Sample_t *adc_sample) {
			adc_sample->real_value_12bits = *(adc_sample->sample);
}

float Supercap_ADC_to_Voltage_Funtion(uint16_t adc_value) {
  float voltage = ((float)adc_value * 3.3f / 4095.0f) * 11;
  return voltage;
}

float Supercap_ADC_to_Voltage_Funtion_Ref(uint16_t adc_value) {
  float voltage = ((float)adc_value * 3.3f / 4095.0f);
  return voltage;
}

float Supercap_ADC_to_Current_Funtion(uint16_t adc_value, uint16_t ref_voltage) {
  // ((ref_12bits*3.3/4095)+(current_flot*0.005)*25)*4095/3.3=current_12bits
  float current = 0.00644689f * (float)(adc_value - ref_voltage);
  return current;
}

//Init
void Supercap_PID_Init(_Supercap_PID_Controller_t *pid_controller, float Kp, float Ki, float Kd, int16_t output_max, int16_t output_min, int16_t I_max) {
  pid_controller->Kp = Kp;
  pid_controller->Ki = Ki;
  pid_controller->Kd = Kd;
  pid_controller->output_max = output_max;
  pid_controller->output_min = output_min;
  pid_controller->I_max = I_max;
}

//Driver
void Supercap_PWM_left(int16_t pwm_duty_cycle) {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_duty_cycle);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 22);
};

void Supercap_PWM_right(int16_t pwm_duty_cycle) {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 278);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pwm_duty_cycle);
};