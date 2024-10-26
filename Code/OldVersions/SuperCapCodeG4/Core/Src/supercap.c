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
	if((*C_left.sample)>=4000 || *(C_right.sample)>=4000){
		temp_counter += 1;
	}
	if(temp_counter >= 5){
		In_loop_PID.output -= 1;
		temp_counter = 0;
	}
}

void TIM2_NVIC(){
  /*
  Supercap_Average_ADC_Function(&C_left);
  Supercap_Average_ADC_Function(&C_sys);
  Supercap_Average_ADC_Function(&C_right);
  Supercap_Average_ADC_Function(&V_sys_op);
  Supercap_Average_ADC_Function(&V_cap_op);
  Supercap_Average_ADC_Function(&V_cap);
  Supercap_Average_ADC_Function(&V_bat);
  Supercap_Average_ADC_Function(&V_sys);
  Supercap_Average_ADC_Function(&V_1V6);5
  Supercap_Average_ADC_Function(&ADC4_3);
  */
 /*
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_left, 8);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_sys, 6); //100kHz
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_right, 4);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_1V6, 2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&ADC4_12,2);
  */

  //set 2, simple test
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_sys, 6);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_right, 4);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&ADC4_12,2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_bat, 2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_cap, 4);
}

void TIM5_NVIC(){
  /*
  Supercap_PID_Controller_Function(&In_loop_PID, &V_cap, Out_loop_PID.output);
	Supercap_PID_Controller_Function(&In_loop_PID, &V_cap, 2600);
  Supercap_PWM_left(In_loop_PID.output);
  */
  //Supercap_PWM_left(100);

  //simple test, outloop
  /*Supercap_PID_Controller_Function(&Out_loop_PID, &C_right, 1894); 
  In_loop_PID.output = (300 * (-Out_loop_PID.output)) / V_bat.real_value_12bits;
  Supercap_PWM_left(In_loop_PID.output);*/
	
	//Simple test, in and out loop
	Supercap_PID_Controller_Function(&Out_loop_PID, &C_sys, 2360, 0.0f); 
  //In_loop_PID.output = (300 * (Out_loop_PID.output)) / V_bat.real_value_12bits;
	Supercap_PID_Controller_Function(&In_loop_PID, &V_cap, Out_loop_PID.output, Out_loop_PID.output * 0.1f);
	//In_loop_PID.output = Out_loop_PID.output * 0.1f; //forward
	Supercap_Soft_Start();
  Supercap_PWM_left(In_loop_PID.output);
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

//Filter
//均方根平均滤波
//明天再写了

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