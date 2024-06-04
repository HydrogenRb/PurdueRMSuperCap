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

//Todo：范例中显示typedef struct _Example_Struct_s Example_Typedef_t {} ，请问_Example_Struct_s是什么？
typedef struct{
  uint8_t count;
  uint16_t average;
  uint32_t history_sum;
  uint16_t *sample;
  float real_value;
}_ADC_Sample_t;

typedef struct{
  float Kp;
  float Ki;
  float Kd;
  float error;
  float error_last;
  float error_sum;
  float output;
  float output_max;
  float output_min;
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
extern _ADC_Sample_t ADC4_3;


float ADC_to_Voltage_Funtion(uint16_t adc_value);
void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample);
void TIM2_NVIC(void);

#ifdef __cplusplus
}
#endif
#endif /*__SUPERCAP_H__ */

