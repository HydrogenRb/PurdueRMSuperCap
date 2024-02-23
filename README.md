# PurdueRMSuperCap (Current: 1.0 testing)
这个文件是为了SuperCap的测试所准备的，其中包含了superCap的测试表以及必要的知识

## Current status


## Table of contents
1.Workflow base on STM32CubeMX and HAL  
2.Introduce of Super Capacitor  
3.Test Bench  

## Part 1- Workflow base on STM32CubeMX and HAL
1.1  
In Purdue Robomaster, we use 
**SWD** -> **MDKARM** -> **ST link** or **CMSIS-DAP** 
to coding the STM32 
The basic of STM32 is GPIO give high or low 
 
1.2  
The basic of STM32CubeMX 
1.2.1 
The type of STM32, STM32F103C8T6 as example 
1.2.2 
The clock source and clock tree 
1.2.3 
The platform of coding 
**Do not for get set this part** 
1.2.4 
How to set the GPIO, find what GPIO we need from PCB board and schematic 
 
1.3  
How to code in Keil5. 
 
1.4  
How to debug and check debuger 
 
1.5  
串口监视器相关 

## Part 2- Intro to Super Cap
2.1 Diagram of Super Cap 
This two diagrams are the sketch for the Super Cap system, you should remember this two sketch! 

2.2 Working Conditions of Super Cap 
First, we turn on the robot, the battery charging the capacitor. 

Then, the robot moving, the battery power the chassis. At this moment, there is no power in or out the capacitor. 

Later, the robot moving fast, the battery and the capacitor power the chassis together. 

At the end, the robot stop moving, we want to inverse power from motor could be charged to capacitors.

In the DCDC system, we can control the voltage by control the open time of four MOSFET. 

2.3 Estimation trasfer function diagram 

## Part 3- 
