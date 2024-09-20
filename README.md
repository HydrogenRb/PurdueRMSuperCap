# PurdueRMSuperCap (Current: 1.0 hardware Finish， documenting) 
This document was created for Super-Capacitor testing and opensource in Purdue Robomaster. This document includes  
1. Preface | 前言
2. Datasheet and design note | 规格书与开源设计笔记
3. Introduce level video | 从零开始的视频教程
4. More information | 更多信息

## Super Cap  
supercapacitor system with 4 MOSFET DC-DC circuits and LM5050 work as a diode. This circuit can charge a capacitor bank with constant power and discharge a capacitor bank with constant voltage. This circuit works with a battery to limit the battery output power.  
  
## Current status
A: 0/6  
B: 0/4  

## Long term target  
1. Change to Altium Designer or KiCAD
2. Reduce the area of PCB
3. Make the STM32 or other microcontroller in the boards  

## Table of contents
1. Workflow based on STM32CubeMX and HAL  
2. Introduce the Super Capacitor  
3. Test Bench  

## Part 1 - Workflow based on STM32CubeMX and HAL[Waiting for finish]
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
**2.1** Diagram of Super Cap  
These two diagrams are the sketches for the Super Cap system, you should remember these two sketches!   
![Overall.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/1%20Overall.png)  
This is the overall diagram, which is complex but helpful.  
![Simple.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/2%20Simple.png)  
This is a simple diagram.  
  
**2.2** Working Conditions of Super Cap  
First, we turn on the robot, the battery charging the capacitor.  
![WorkingConditionCharge.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/3%20WorkingCondition_Charge.png)  
Then, the robot moves, and the battery powers the chassis. At this moment, there is no power in or out of the capacitor.  
![WorkingConditionIdle.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/4%20WorkingCondition_Idle.png)  
Later, the robot moves fast, and the battery and the capacitor power the chassis together.  
![WorkingConditionDischarge.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/5%20WorkingCondition_Discharge.png)  
In the end, the robot stops moving, we want to inverse power from the motor that could be charged to capacitors.  
![WorkingConditionInverseCharge.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/6%20WorkingCondition_InverseCharge.png)  
In the DCDC system, we can control the voltage by controlling the open time of four MOSFETs.  
![ControlDiagram.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/7%20ControlDiagram.png)  
**2.3** Estimation transfer function diagram  
![DiagramWithLabel.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/8%20DiagramWithLabel.png)  
![TransferFunction.png](https://github.com/HydrogenRb/PurdueRMSuperCap/blob/main/Images/9%20TransferFunction.png)  
We need to design the control system for the supercap. (AKA, the parameters in the blocks) 
We follow some rules like:  
If I1 is too high, we reduce the I3 to a negative value.  
If I2 is too high, we reduce the I3.  
...  

2.4 Power tree [waiting for finish]
There are several ICs in this system, we need different voltages to make them work. The power tree is shown below. 
## Part 3 - Test bench
### Part A. Function of the circuit  
A.1 **Power on** testing: Check if something is wrong under 24V. Check all auxiliary voltages are correct.  
A.2 **Diode** testing: Check if the MOSFET diode (OR-ing Controller based on LM5050) works well under 3V, 10V, and 24V.  
A.3 **Voltage sampling** testing: Test the ADC for the voltage divider (ADC3 and ADC4).  
A.4 **Current sampling resistor** testing: Check the voltage across the current sampling resistor (5mΩ resistor).  
A.5 **Current sampling circuit** testing: Check the current sampling circuit work (ADC0, ADC1, and ADC2) (INA181).  
A.6 **MOSFET control** testing: Verify that we can control the opening or close of MOSFET by STM32.  
### Part B. Function of charging  
B.1 **Basic BUCK** testing: Test that the buck circuit can make 5V to 2V. Monitor the efficiency of the load. Monitor the ripple by oscilloscope. (Note: Remember to set the maximum output current of the adjustable power supply to prevent damage to the MOSFET)  
B.2 **Overall BUCK** testing: Reduce the 24V to 2V, 10V, and 15V. Monitor the efficiency of the load. Monitor the ripple by oscilloscope. Try to reduce the ripple.  
B.3 **BUCK limit the current** testing: Design the BUCK circuit to limit the current in 1A under different loads.  
B.4 **1A charging** testing: Test the charge of real capacitors.  
### Part C. Function of discharging  
C.1 **Basic BOOST** testing: Test that the buck circuit rises from 5V to 10V. Monitor the efficiency by load. Monitor the ripple by oscilloscope.  
C.2 **Overall BOOST** testing: Rising the 20V, 15V, 5V, and 3V to 24V (5V and 3V test is not necessary).  
C.3 **High Current BOOST** testing: Same testing with C.2, but the output current will be 2A.  (Note: I need to review the current limit of the MOSFETs)  
### Part D. Whole testing
D.1 **Charging** testing: test the working condition-charge  
D.2 **Idle** testing: test the working condition-Idle  
D.3 **Discharge** testing: test the working condition-discharge  
D.4 **InverseCharge** testing: test the working condition-InverseCharge (need to do more research on this).  
### Part E. Communication testing  
E.1 **Communication** testing: This system can read data from the referee system and store power limitations in one variable. (Note: you can just copy from Leo)  
E.2 **Changeing power** testing: This system can change the output power depending on the variable from E.1.  

## Part 4 - Timeline [waiting]
Week of Feb 25: Finish A1 and A2,
Week of Feb 25: Finish the abstract.
