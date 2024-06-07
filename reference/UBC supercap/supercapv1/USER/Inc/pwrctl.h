#ifndef _PWRCTL_H
#define _PWRCTL_H

#include <math.h>
#include <stdint.h>

#include "main.h"
#include "adc.h"
#include "usart.h"

#define PWM_RELOAD 399
#define PWM_MINVAL 10

#define ADC_MAX 4095.0f
#define VOLTAGE_SENSE_RATIO 10.09091f
#define VREF_VAL 3.3f



#define INA_AMPLIFY_RATIO 50.0f
#define SAMPLING_RES (0.002f)

#define IIR_V 0.005f
#define IIR_I 0.5f
#define IIR_P_DISPLAY 0.1f

/************SAFETY SETTINGS**********/
#define BUS_UVP_THRE 17.0f
#define BUS_OVP_THRE 27.0f
#define BAT_OVP_THRE 30.0f
#define BAT_FULL_VOL 23.5f
#define BAT_UVP_STARTUP_THRE 10.0f

#define PROTECTION_RECOVERY_TIME 2000
/*************************************/

#define DT (((float)PWM_RELOAD+1.0f)/170000000.0f)

#define VOFA_TAIL {0x00, 0x00, 0x80, 0x7f}
typedef struct pwr_data_t
{
    float v_bus;
    float i_bus;
    float v_cap;
    float i_cap;
    float i_base;
    float state;
    float p_bus;
    float duty;
    float testval;
    unsigned char tail[4];
}pwr_data_t;

/**
 * @brief direct value from ADC
 */
typedef struct pwr_adc_t
{
    uint16_t v_bus;//adc1 in3 (PA2)
    uint16_t i_bus;//adc1 in4 (PA3)
    uint16_t v_refint;//adc1 Vrefint
    uint16_t i_cap;//adc2 in1 (PA0)
    uint16_t v_cap;//adc2 in2 (PA1)
    uint16_t i_base;//adc2 in17 (PA4)
}PWR_adc_t;

typedef struct pid_t
{
    const float p;
    const float i;
    const float d;
    const float i_max;
    float errm1;
    float err_i;
}PID_t;

enum Cap_states{
    CAP_OFF,
    CAP_READY,
    CAP_ON,
    VBUS_OVP,
    VBUS_UVP,
    VBAT_OVP,
};

void pwrctl_init();
void setTargetPower(float power);
void pwrctl_ISR();

#endif
