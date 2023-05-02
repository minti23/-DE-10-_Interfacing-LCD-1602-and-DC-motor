# DE-10_Interfacing-LCD-1602-and-DC-motor

----------------------------------------------------------------------------------------------
    
- Build a system using Nios II in kit DE10 to connect a LCD 16x2 and an
H-bridge to control a motor. This system can do the following tasks:
 + When SW0 is ON, LCD blinks the sentence “Hello World !!!” in the
 Middle of row 1 with frequency 1Hz. (Using timer)
 + When SW1 is ON, Nios II controls the motor by sending PWM pulses to
 The H-bridge. LCD displays the duty cycle and the frequency of PWM
 pulses.
 + When SW0 and SW1 are OFF, turn off the system.
- EXTENSION:
 + SW1, SW2, SW3 will control the speed of the DC motor based on the PWM
 Pulses be created by DE-10 kit nano.
 
----------------------------------------------------------------------------------------------

--> File miniProject.qsys to set the system on NiosII

--> File mini.v and folder software to connect gpio and create commands to excute

