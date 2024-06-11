/**
  ******************************************************************************
  * @file    supercap.c
  * @brief   This file provides code for the configuration
  *          of all used supercap pins.
  */
/* USER CODE END Header */
#include "supercap.h"

uint8_t PID_flag = 0;
uint16_t protection_counter = 0;
uint16_t fault_counter = 0;

//Timmer
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance == TIM5){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    Supercap_FSM();
    TIM5_NVIC();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	}
  else if(htim->Instance == TIM2) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    TIM2_NVIC();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
  }
}

void TIM2_NVIC(){
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_sys, 6);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&C_right, 4);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&ADC4_12,2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_bat, 2);
  Supercap_First_Order_Filter_ADC_Function_Alpha(&V_cap, 4);
}

void TIM5_NVIC(){
	if(PID_flag == 1){
	Supercap_PID_Controller_Function(&PID_45W_loop, &C_sys, CURRENT_LIMIT, ((CURRENT_LIMIT-C_sys.real_value_12bits)*5.0f+V_cap.real_value_12bits));
	Supercap_PID_Controller_Function(&PID_n7A_loop, &C_right, 3100, V_cap.real_value_12bits);
  Supercap_PID_Controller_Function(&PID_7A_loop, &C_right, 1000, V_cap.real_value_12bits);
  temp2 = Supercap_Compare(PID_45W_loop.output, PID_n7A_loop.output, PID_7A_loop.output);
	Supercap_PID_Controller_Function(&PID_voltage_loop, &V_cap, temp2, 0.07f*temp2);
  temp2 = Supercap_Limit(temp2, 2707, 0); //should I have voltage limit?
	Supercap_PWM_left(temp2*0.1f);
		}
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

int16_t Supercap_Limit(int16_t input, int16_t max, int16_t min){
  if (input > max) {
    return max;
  } else if (input < min) {
    return min;
  } else {
    return input;
  }
}

void Supercap_FSM(){
  //Protection
  if(V_cap.real_value_12bits < VOLTAGE_LIMIT && C_sys.real_value_12bits < CURRENT_LIMIT){
		fault_counter++;
		if(fault_counter >= 100){
			Supercap_AUX_BlueLED(60);
			Supercap_AUX_YellowLED(60);
      Supercap_AUX_MachineSpirit(0);
			Supercap_DCDC_Stop();
			CAP_STATE = FAULT;
			fault_counter = 0;
		}
  } else{
		//fault_counter--;
	}
  
  if(C_right.real_value_12bits > 4063 || C_right.real_value_12bits < 31){
		fault_counter++;
		if(fault_counter >= 2){
			Supercap_AUX_BlueLED(60);
			Supercap_AUX_YellowLED(60);
      Supercap_AUX_MachineSpirit(0);
			Supercap_DCDC_Stop();
			CAP_STATE = FAULT;
			fault_counter = 0;
		}
  } else{
		fault_counter--;
	}
  if(C_right.real_value_12bits > 3443 || C_right.real_value_12bits < 651){
		fault_counter++;
		if(fault_counter >= 30){
			Supercap_AUX_BlueLED(60);
			Supercap_AUX_YellowLED(60);
      Supercap_AUX_MachineSpirit(0);
			Supercap_DCDC_Stop();
			CAP_STATE = FAULT;
			fault_counter = 0;
		}
  }	

  //State switch
  switch(CAP_STATE){
    case INIT:
      if(V_cap.real_value_12bits > 1){
        Supercap_AUX_YellowLED(60);
				Supercap_AUX_BlueLED(0);
        Supercap_AUX_MachineSpirit(0);
        CAP_STATE = READY;
        }
    case READY:
      if(V_cap.real_value_12bits > 50){
        CAP_STATE = RUNNING;
				Supercap_AUX_YellowLED(0);
				Supercap_AUX_BlueLED(0);
        Supercap_AUX_MachineSpirit(60);
				Supercap_DCDC_Start();
      }
      break;
    case RUNNING:
      if(V_bat.real_value_12bits < 2256 && C_sys.real_value_12bits < 2047){
        CAP_STATE = STOP;
				Supercap_DCDC_Stop();
        Supercap_AUX_MachineSpirit(0);
				Supercap_AUX_BlueLED(0);
				Supercap_AUX_YellowLED(0);
      }
      break;
    case STOP:
			if(V_bat.real_value_12bits > 2256 && C_sys.real_value_12bits > 2047){
        CAP_STATE = READY;
				Supercap_AUX_BlueLED(0);
				Supercap_AUX_YellowLED(80);
        Supercap_AUX_MachineSpirit(0);
			}
      break;
    case FAULT:
			if(protection_counter >= 2000 && C_sys.real_value_12bits < CURRENT_LIMIT){
				CAP_STATE = READY;
				Supercap_AUX_BlueLED(0);
				Supercap_AUX_YellowLED(80);
        Supercap_AUX_MachineSpirit(0);
			}
      break;
  }

  //State output
  switch(CAP_STATE){
		case INIT:
			PID_flag = 0;
      protection_counter = 0;
			break;
    case READY:
      PID_flag = 0;
      protection_counter = 0;
      break;
    case RUNNING:
			PID_flag = 1;
      break;
    case STOP:
			PID_flag = 0;
      break;
    case FAULT:
      Supercap_PID_Reset(&PID_45W_loop);
      Supercap_PID_Reset(&PID_n7A_loop);
      Supercap_PID_Reset(&PID_7A_loop);
      Supercap_PID_Reset(&PID_voltage_loop);
      protection_counter++;
      break;
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

void Supercap_PID_Reset(_Supercap_PID_Controller_t *pid_controller){
  pid_controller->error_sum = 0;
}

//Driver
void Supercap_PWM_left(int16_t pwm_duty_cycle) {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_duty_cycle);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 22);
}

void Supercap_PWM_right(int16_t pwm_duty_cycle) {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 278);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pwm_duty_cycle);
}

void Supercap_PWM_GND() {
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 300);
}

void Supercap_DCDC_Start(){
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_4);
}

void Supercap_DCDC_Stop(){
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_4);
}

void Supercap_AUX_Init(){
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  TIM3_AUTORELOAD_over100 = 480;
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //Buzzer
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //LED_0 red
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); //LED_1 yellow
	
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); //Machine Spirit
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); //LED blue
}

void Supercap_AUX_RedLED(uint32_t brightness){
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, brightness * TIM3_AUTORELOAD_over100);
}

void Supercap_AUX_YellowLED(uint32_t brightness){
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, brightness * TIM3_AUTORELOAD_over100);
}

void Supercap_AUX_BlueLED(uint32_t brightness){
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, brightness * 480);
}

void Supercap_AUX_MachineSpirit(uint32_t brightness){
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, brightness * 480);
}

void Supercap_AUX_Buzzer(uint32_t volume, int16_t note){
if (note == -1) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 11538);
  TIM3_AUTORELOAD_over100 = 115;
} else if (note == -2) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 10273);
  TIM3_AUTORELOAD_over100 = 103;
} else if (note == -3) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 9146);
  TIM3_AUTORELOAD_over100 = 91;
} else if (note == -4) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 8620);
  TIM3_AUTORELOAD_over100 = 86;
} else if (note == -5) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 7692);
  TIM3_AUTORELOAD_over100 = 77;
} else if (note == -6) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 6818);
  TIM3_AUTORELOAD_over100 = 68;
} else if (note == -7) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 6097);
  TIM3_AUTORELOAD_over100 = 61;
} else if (note == 1) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 5747);
  TIM3_AUTORELOAD_over100 = 57;
} else if (note == 2) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 5119);
  TIM3_AUTORELOAD_over100 = 51;
} else if (note == 3) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 4559);
  TIM3_AUTORELOAD_over100 = 46;
} else if (note == 4) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 4297);
  TIM3_AUTORELOAD_over100 = 43;
} else if (note == 5) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 3836);
  TIM3_AUTORELOAD_over100 = 38;
} else if (note == 6) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 3409);
  TIM3_AUTORELOAD_over100 = 34;
} else if (note == 7) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 3042);
  TIM3_AUTORELOAD_over100 = 30;
} else if (note == 11) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 2868);
  TIM3_AUTORELOAD_over100 = 29;
} else if (note == 12) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 2555);
  TIM3_AUTORELOAD_over100 = 26;
} else if (note == 13) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 2276);
  TIM3_AUTORELOAD_over100 = 23;
} else if (note == 14) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 2148);
  TIM3_AUTORELOAD_over100 = 21;
} else if (note == 15) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 1915);
  TIM3_AUTORELOAD_over100 = 19;
} else if (note == 16) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 1704);
  TIM3_AUTORELOAD_over100 = 17;
} else if (note == 17) {
  __HAL_TIM_SET_AUTORELOAD(&htim3, 1518);
  TIM3_AUTORELOAD_over100 = 15;
} else{
  __HAL_TIM_SET_AUTORELOAD(&htim3, 1000);
  TIM3_AUTORELOAD_over100 = 10;
}
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, volume * TIM3_AUTORELOAD_over100);
}