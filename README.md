# PurdueRMSuperCap (Current: 1.0 testing) 
This document created for Super Capacitor testing in Purdue Robomaster. This document include 
这个文件是为了SuperCap的测试所准备的，其中包含了superCap的测试表以及必要的知识

## Current status


## Table of contents
1. Workflow based on STM32CubeMX and HAL  
2. Introduce the Super Capacitor  
3. Test Bench  

## Part 1 - Workflow based on STM32CubeMX and HAL
1.1  
In Purdue Robomaster, we use 
**SWD** -> **MDKARM** -> **ST link** or **CMSIS-DAP** 
to code the STM32 
The basis of STM32 is GPIO gives high or low 
 
1.2  
The basics of STM32CubeMX 
1.2.1 
The type of STM32, STM32F103C8T6 as example 
1.2.2 
The clock source and clock tree 
1.2.3 
The platform for coding 
**Do not forget set this part** 
1.2.4 
How to set the GPIO, find what GPIO we need from the PCB board and schematic 
 
1.3  
How to code in Keil5. 
 
1.4  
How to debug and check the debugger 
 
1.5  
串口监视器相关 

## Part 2 - Intro to Super Cap
2.1 Diagram of Super Cap 
These two diagrams are the sketches for the Super Cap system, you should remember these two sketches!  
[Images/1 Overall.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/1%20Overall.png)) 

2.2 Working Conditions of Super Cap 
First, we turn on the robot, the battery charging the capacitor. 

Then, the robot moves, and the battery powers the chassis. At this moment, there is no power in or out of the capacitor. 

Later, the robot moves fast, and the battery and the capacitor power the chassis together. 

In the end, the robot stops moving, we want to inverse power from the motor that could be charged to capacitors.

In the DCDC system, we can control the voltage by controlling the open time of four MOSFETs. 

2.3 Estimation transfer function diagram 
We need to design the control system for the supercap. (AKA, the parameters in the blocks) 
We follow some rules like: 
If I1 is too high, we reduce the I3 to a negative value. 
If I2 is too high, we reduce the I3. 
... 

2.4 Power tree 
There are several ICs in this system, we need different voltages to make them work. The power tree is shown below. 
## Part 3 - Test bench
### Part A. Function of the circuit 
A.1 Power on testing: Check if something is wrong under 24V. Check the 
### Part B. Function of charging
### Part C. Function of discharging
### Part D. Whole testing
### Part E. Communication testing

## Part 4 - Timeline
