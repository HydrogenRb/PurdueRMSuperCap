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

enum _CAP_STATE_T {READY = 1, RUNNING, STOP, FAULT, INIT};

extern uint16_t supercap_ADC1[2];
extern uint16_t supercap_ADC2[3];
extern uint16_t supercap_ADC3[3];
extern uint16_t supercap_ADC4[2];
extern uint16_t supercap_max_power_current;
extern uint16_t supercap_max_power_STM32;
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

extern _Supercap_PID_Controller_t PID_45W_loop;
extern _Supercap_PID_Controller_t PID_n7A_loop;
extern _Supercap_PID_Controller_t PID_7A_loop;
extern _Supercap_PID_Controller_t PID_voltage_loop;

extern uint8_t TxData[1];

extern float temp2;
extern float supercap_target_voltage;
extern uint16_t temp_counter;

extern uint32_t TIM3_AUTORELOAD_over100;
extern enum _CAP_STATE_T CAP_STATE;

extern uint32_t debug_counter;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void TIM6_NVIC();
void TIM2_NVIC();
void TIM5_NVIC();
uint16_t Update_Current(uint16_t old_current, uint16_t new_current);

void Supercap_PID_Controller_Function(_Supercap_PID_Controller_t *pid_controller, _ADC_Sample_t *adc_sample, int16_t target, float offset);
void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample);
void Supercap_Soft_Start(void);
void Supercap_First_Order_Filter_ADC_Function(_ADC_Sample_t *adc_sample);
void Supercap_First_Order_Filter_ADC_Function_Alpha(_ADC_Sample_t *adc_sample, uint16_t alpha);
float Supercap_ADC_to_Voltage_Funtion(uint16_t adc_value);
float Supercap_ADC_to_Voltage_Funtion_Ref(uint16_t adc_value);
float Supercap_ADC_to_Current_Funtion(uint16_t adc_value, uint16_t ref_voltage);
void Supercap_Function_Init(_ADC_Sample_t *adc_sample);
void Supercap_PID_Init(_Supercap_PID_Controller_t *pid_controller, float Kp, float Ki, float Kd, int16_t output_max, int16_t output_min, int16_t I_max);
void Supercap_PID_Reset(_Supercap_PID_Controller_t *pid_controller);
int16_t Supercap_Compare(int16_t Output1, int16_t Output2, int16_t Output3);
int16_t Supercap_Limit(int16_t input, int16_t max, int16_t min);
void Supercap_FSM();
void Supercap_PWM_left(int16_t pwm_duty_cycle);
void Supercap_PWM_right(int16_t pwm_duty_cycle);
void Supercap_PWM_GND();
void Supercap_DCDC_Start();
void Supercap_DCDC_Stop();
void Supercap_AUX_Init();
void Supercap_AUX_RedLED(uint32_t brightness);
void Supercap_AUX_YellowLED(uint32_t brightness);
void Supercap_AUX_BlueLED(uint32_t brightness);
void Supercap_AUX_MachineSpirit(uint32_t brightness);
void Supercap_AUX_Buzzer(uint32_t volume, int16_t note);

#ifdef __cplusplus
}
#endif
#endif /*__SUPERCAP_H__ */
