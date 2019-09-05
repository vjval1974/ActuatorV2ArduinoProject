#include <Arduino.h>
#include "./StateMachines/ActuatorStateMachine.h"
#include "./Pushbuttons/FsrPushbutton.h"

/*
Mappings dont matter here (I did them anyway). 
The integer value of the pin is mapped in code therefore we 
only need to use: pinMode(6, OUTPUT) for example
or digitalRead(6);
Note: A0,A1,A2,A3 should be used to read analog values
PORT D 
0	PIN 2 
1	PIN 3
2
3
4	PIN 4
5   LED 
6
7   PIN 6

PORT C 
6 	PIN 5

PORT B 
0	LED 
1	PIN 15
2	PIN 16
3	PIN 14
4	PIN 8
5	PIN 9
6	PIN 10
7 

ASSIGNMENTS 
============
-------------------------------------------------------------------------
PIN # 			 | 	description										    |
(on pro micro)   |														|
-------------------------------------------------------------------------
PIN 2				Motor Controller fwd						
PIN 3				Motor Controller bwd
PIN 4				Motor Controller stop
PIN 5 				Motor Controller speed2
PIN 6				Motor Controller fault	
PIN 8				Suction Cup solenoid
PIN 9				Suction Cup position sensor
PIN 10 				Vacuum Pressure switch
PIN 14				Vacuum Solenoid
PIN 15 
PIN 16
PIN A0				Start Pushbutton
PIN A1 				Stop Pushbutton
PIN A2				Touch Sensor	
PIN A3
*/


ActuatorStateMachine actuatorStateMachine = ActuatorStateMachine();
FsrPushbutton testPushbutton1 = FsrPushbutton(A0, 5);
FsrPushbutton testPushbutton2 = FsrPushbutton(A2, 5);

void setup()
{
	Serial.begin(115200);
		
	testPushbutton1 = FsrPushbutton(A0, 5);
	testPushbutton2 = FsrPushbutton(A2, 5);

	delay(1000);
	
	Serial.println("Started");
	
}

void loop()
{
	//actuatorStateMachine.Process();


	testPushbutton1.PollPresses();
	testPushbutton2.PollPresses();

	PressState tpb1State = testPushbutton1.IsPress();
	PressState tpb2State = testPushbutton2.IsPress();
	if (tpb1State == SINGLE_PRESS)
		Serial.println("Button 1: SINGLE PRESS: ");
	if (tpb1State == DOUBLE_PRESS)
		Serial.println("Button 1: DOUBLE PRESS: ");

	if (tpb2State == SINGLE_PRESS)
		Serial.println("Button 2: SINGLE PRESS: ");
	if (tpb2State == DOUBLE_PRESS)
		Serial.println("Button 2: DOUBLE PRESS: ");


	delay(100);
	
}
