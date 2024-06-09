/**
 * @file Serial.h
 * @author Leo Liu
 * @brief Serial Communication with Laptop
 * @version 1.0
 * @date 2023-10-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __SERIAL_H
#define __SERIAL_H
 
#include <stdio.h>
#include <stdint.h>
#include "usart.h"

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart3,(uint8_t*)&ch,1,0xFFFF);
	return ch;
}

#endif
