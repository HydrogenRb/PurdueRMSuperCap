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


float ADC_to_Voltage_Funtion(uint16_t adc_value);
void Supercap_Average_ADC_Function(_ADC_Sample_t *adc_sample);

#ifdef __cplusplus
}
#endif
#endif /*__SUPERCAP_H__ */

