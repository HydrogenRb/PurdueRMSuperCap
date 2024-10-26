/**
  ******************************************************************************
  * @file    supercap.h
  * @brief   This file contains all the function prototypes for
  *          the supercap.c file
  */
#ifndef __SUPERCAP_H__
#define __SUPERCAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "Kalman_Filter.h"

//Todo：范例中显示typedef struct _Example_Struct_s Example_Typedef_t {} ，请问_Example_Struct_s是什么？
typedef struct{
  uint16_t *sample;
  uint8_t count;
  uint16_t average;
  uint32_t history_sum;
	uint16_t real_value_12bits;
	float real_value_float;
}_ADC_Sample_t;

typedef struct{
  float Kp;
  float Ki;
  float Kd;
  int16_t error;
  int16_t error_last;
  float error_sum;
  int16_t output;
  int16_t output_max;
  int16_t output_min;
	int16_t I_max;
}_Supercap_PID_Controller_t;

extern uint16_t supercap_ADC1[2];
extern uint16_t supercap_ADC2[3];
extern uint16_t supercap_ADC3[3];
extern uint16_t supercap_ADC4[2];
extern _ADC_Sample_t C_left;
extern _ADC_Sample_t C_sys;
extern _ADC_Sample_t C_right;
extern _ADC_Sample_t V_sys_op;
extern _ADC_Sample_t V_cap_op;
extern _ADC_Sample_t V_cap; //V_right
extern _ADC_Sample_t V_bat; //V_left
extern _ADC_Sample_t V_sys;
extern _ADC_Sample_t V_1V6;
extern _ADC_Sample_t ADC4_12;
extern Kalman_Filter_t C_left_kalman;

extern _Supercap_PID_Controller_t Out_loop_PID;
extern _Supercap_PID_Controller_t In_loop_PID;

extern float temp2;
extern uint16_t temp_counter;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void TIM2_NVIC();
void TIM5_NVIC();

void Supercap_PID_Controller_Function(_Supercap_PID_Controller_t *pid_controller, _ADC_Sample_t *adc_sample, int16_t target, float offset);
void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample);
void Supercap_Soft_Start(void);
void Supercap_First_Order_Filter_ADC_Function(_ADC_Sample_t *adc_sample);
void Supercap_First_Order_Filter_ADC_Function_Alpha(_ADC_Sample_t *adc_sample, uint16_t alpha);
float Supercap_ADC_to_Voltage_Funtion(uint16_t adc_value);
float Supercap_ADC_to_Voltage_Funtion_Ref(uint16_t adc_value);
float Supercap_ADC_to_Current_Funtion(uint16_t adc_value, uint16_t ref_voltage);
void Supercap_PID_Init(_Supercap_PID_Controller_t *pid_controller, float Kp, float Ki, float Kd, int16_t output_max, int16_t output_min, int16_t I_max);
void Supercap_PWM_left(int16_t pwm_duty_cycle);
void Supercap_PWM_right(int16_t pwm_duty_cycle);

#ifdef __cplusplus
}
#endif
#endif /*__SUPERCAP_H__ */
