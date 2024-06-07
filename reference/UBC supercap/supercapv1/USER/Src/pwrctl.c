/**
 * @file pwrctl.c
 * @author Liwei Xue (luo@tianyi.vc)
 * @date 2024-06-05
 * @copyright Copyright (c) Liwei Xue 2024
 * 
 * @attention DO NOT change ANY of this code unless fully tested.
 * Inappropriate change may cause hardware damage.
 * 在没有充分理解并进行使用硬件测试前，请不要更改任何代码。对控制环路的不恰当
 * 更改可能导致系统失控/震荡，乃至损坏电路元件。
 */
#include "pwrctl.h"

static float target_current=1.0f;

/* --------- 请根据需要修改这里。 ----- */
static float target_power=63.0f; //期望底盘功率。
// 注意，模块识别的功率与裁判系统之间存在误差，上场前请校正。
/* ---------------------------------- */

static PWR_adc_t adc_val;
pwr_data_t data={.v_bus=2.333f, .tail=VOFA_TAIL};

int state=CAP_OFF;
uint32_t protection_triggered=0;

PID_t _i={
    .p=2.0f,
    .i=60000.0f,
    .d=0.00005f,
    .i_max=0.001f,
    .errm1=0.0f,
    .err_i=0.0f,
};

/**
 * @brief Turn on the MOS driver circuit, enable DCDC. 
 * 开启DCDC电路。
 */
void dcdc_on(){
    HAL_GPIO_WritePin(ENL_GPIO_Port, ENL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ENR_GPIO_Port, ENR_Pin, GPIO_PIN_SET);
}
/**
 * @brief Turn off the MOS driver circuit, disable DCDC. 
 * 关闭DCDC电路。
 */
void dcdc_off(){
    HAL_GPIO_WritePin(ENL_GPIO_Port, ENL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ENR_GPIO_Port, ENR_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Reset integral term of DCDC. 
 * 重置积分项。
 */
static void pid_reset(){
    _i.err_i= 0;
}

/**
 * @brief Initialize PWM, start operation. 
 * 初始化DCDC外设。
 */
void pwrctl_init(){
    dcdc_off();

    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH4);
    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_EnableAllOutputs(TIM2);
    LL_TIM_EnableCounter(TIM1);
    LL_TIM_EnableCounter(TIM2);
    LL_TIM_EnableCounter(TIM3);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&(adc_val.v_bus), 3);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&(adc_val.i_cap), 3);

    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)&data, sizeof(data));
    state=CAP_READY;
    LL_TIM_EnableIT_UPDATE(TIM2);
}

/**
 * @brief Set target total power.
 * 设置期望底盘总功率。
 * @param power unit in w
 */
void setTargetPower(float power){
    power*=0.98f;
    if(power<2.0f) power=2.0f;
    if(power>200.0f) power=200.0f;
}

/**
 * @brief Set target total current. 
 * @attention This function will be AUTOMATICLY called
 * 
 * @param current unit in A
 */
void setTargetCurrent(float current){
    if(current<0.0f) current=0.0f;
    target_current=current;

    if(data.v_cap > BAT_FULL_VOL - 0.5f){
        target_current*= (BAT_FULL_VOL-data.v_cap)*(1.0f/0.5f);
    }
}

/*
L pwm -> T1C1
R pwm -> T2C4
*/

/**
 * @brief Update PWM duty of dcdc
 * 
 * @param duty the reference direction is BUS -> CAP
 */
static void update_duty(float duty){
    data.duty=duty;
    duty=100.0f-duty;//convert direction to CAP -> BUS

    /* 限制低电压下放电占空比，防止陷入过放状态。 */
    float duty_max=100.0f;
    if(data.v_cap < BAT_UVP_STARTUP_THRE){
        duty_max=(data.v_cap)/(BAT_UVP_STARTUP_THRE)*70.0f+3.0f;
    }

    if(duty<0.0f)   duty=0.0f;
    if(duty>100.0f) duty=100.0f;
    if(duty>duty_max) duty=duty_max;

    int lduty, rduty;
    if(duty<50.0f){ //buck mode
        rduty=PWM_RELOAD-PWM_MINVAL;
        lduty=PWM_MINVAL+(int)(duty*(float)(PWM_RELOAD-2*PWM_MINVAL)*(1.0f/50.0f));
    }else{
        rduty=PWM_MINVAL+(int)((100.0f-duty)*(float)(PWM_RELOAD-2*PWM_MINVAL)*(1.0f/50.0f));
        lduty=PWM_RELOAD-PWM_MINVAL;
    }

    LL_TIM_OC_SetCompareCH1(TIM1, lduty);
    LL_TIM_OC_SetCompareCH4(TIM2, rduty);
}

/**
 * @brief Update data based on ADC readings.
 * 
 * @note 1st order IIR low-pass filters are applied to all readings.
 * See pwrctl.h for filter constants.
 */
static void update_data(){
    
    float v_bus=(float)adc_val.v_bus*(VREF_VAL*VOLTAGE_SENSE_RATIO/ADC_MAX);
    data.v_bus=data.v_bus*(1.0f-IIR_V)+v_bus*IIR_V;

    float v_cap=(float)adc_val.v_cap*(VREF_VAL*VOLTAGE_SENSE_RATIO/ADC_MAX);
    data.v_cap=data.v_cap*(1.0f-IIR_V)+v_cap*IIR_V;

    float v_ibus=(float)adc_val.i_bus*(VREF_VAL/ADC_MAX);
    float i_bus=(v_ibus-2.5f)*(1.0f/(INA_AMPLIFY_RATIO*SAMPLING_RES));
    //data.i_bus=data.i_bus*(1.0f-IIR_I)+i_bus*IIR_I;
    //data.i_bus=v_ibus;

    float v_icap=(float)adc_val.i_cap*(VREF_VAL/ADC_MAX);
    float i_cap=(v_icap-2.5f)*(1.0f/(INA_AMPLIFY_RATIO*SAMPLING_RES));
    data.i_cap=data.i_cap*(1.0f-IIR_I)+i_cap*IIR_I;

    float v_ibase=(float)adc_val.i_base*(VREF_VAL/ADC_MAX);
    float i_base=(1.25f-v_ibase)*(1.0f/(INA_AMPLIFY_RATIO*SAMPLING_RES*1.17f));
    data.i_base=data.i_base*(1.0f-IIR_I)+i_base*IIR_I;

    data.i_bus=data.i_base*0.1f;

    data.p_bus=data.p_bus*(1.0f-IIR_P_DISPLAY)+data.i_base*data.v_bus*IIR_P_DISPLAY;
    
}

/**
 * @brief Calculate PID for next period.
 * @note This PID controls 
 */
static void update_pid(){
    float i_err=target_current-data.i_base;
    _i.err_i += i_err*DT;
    float err_d=(i_err-_i.errm1)*(1.0f/DT);
    _i.errm1=i_err;

    if(_i.err_i > _i.i_max) _i.err_i=_i.i_max;
    if(_i.err_i < -_i.i_max) _i.err_i=-_i.i_max;
    
    update_duty(i_err*_i.p + _i.err_i*_i.i + err_d*_i.d);
}

/**
 * @brief State machine implementation of DCDC module.
 * 
 */
static void cap_state_machine(){
    data.state=(float)state*10;

    if(data.v_bus > BUS_OVP_THRE){ //BUS over-voltage protection
        state=VBUS_OVP;
        protection_triggered=HAL_GetTick();
    }else if(data.v_bus < BUS_UVP_THRE){ //BUS under-voltage halt
        state=VBUS_UVP;
    }else if(data.v_cap > BAT_OVP_THRE){ //BAT over-voltage protection
        state=VBAT_OVP;
        data.testval=data.v_cap;// debug display output
        protection_triggered=HAL_GetTick();
    }else if(state==VBUS_UVP && data.v_bus > BUS_UVP_THRE \
        && protection_triggered < HAL_GetTick() - 10){ 
        //recovery from BUS UVP
        pid_reset();
        protection_triggered=HAL_GetTick();
        state=CAP_READY;
    }

    //state-transition of DCDC FSM
    switch (state)
    {
    case CAP_ON:
        update_pid();
        break;
    case CAP_READY:
        dcdc_on();
        state=CAP_ON;
        break;
    case CAP_OFF: //本行疑似bug。暂时似乎没有影响。
    case VBUS_OVP:
    case VBAT_OVP:
    //进入保护后，系统会每隔 PROTECTION_RECOVERY_TIME 毫秒尝试重启。
        if(HAL_GetTick() > protection_triggered + PROTECTION_RECOVERY_TIME){
            pid_reset();
            state=CAP_READY;
        }
    case VBUS_UVP:
        dcdc_off();
        break;
    default:
        state=CAP_OFF;
    }
}

/**
 * @brief Interupt serivce request of control algorithm. 
 * 控制器的中断服务函数。在stm32g4xx_it.c中被调用。
 */
void pwrctl_ISR(){
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,GPIO_PIN_SET);
    update_data();

    setTargetCurrent(target_power/data.v_bus);
    cap_state_machine();
    
    
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,GPIO_PIN_RESET);
}
