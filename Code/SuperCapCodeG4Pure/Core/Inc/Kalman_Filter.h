/**
 * @file Kalman Filter.c
 * @author Leo Liu
 * @brief Kalman Filter Algorithm Library
 * @version 1.0
 * @date 2023-09-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __KALMAN_FILTER_H
#define __KALMAN_FILTER_H

#include <stdio.h>
#include <stdint.h>

typedef struct
{
	float Prev_P;
	float Current_P;
	float Output;
	float K;
	float Q;
	float R;
}Kalman_Filter_t;

	float First_Order_Kalman_Filter(Kalman_Filter_t *KF, float Measurement);

#endif
