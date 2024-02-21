# PurdueRMSuperCap (Current: 1.0 testing)
这个文件是为了SuperCap的测试所准备的，其中包含了superCap的测试表以及必要的知识

## Current status


## Table of contents
1.Workflow base on STM32CubeMX and HAL  
2.Introduce of Super Capacitor  
3.Test Bench  

## Part 1- Workflow base on STM32CubeMX and HAL
1.1  
In Purdue Robomaster, we use **SWD** -> **MDKARM** -> **ST link** or **CMSIS-DAP** to coding the STM32
1.2  
The basic of STM32CubeMX
1.3  
How the STM32 works? How to code in Keil5.
1.4  
How 
1.5  

2.362学生可以跳过：我们控制芯片输出高低电压，然后板子上面的电路和接口配合着输出各式各样的东西（比如配合蜂鸣器+MOS管可以输出声音，配合LED可以闪光）
3.CubeMX创建教程，我们主要要使用的有ADC、GPIO、TIM、UART
4.keil5编程、上传教程（小魔法棒）
5.给362学生：如果想要使用串口监视器，可以单独下载后使用
