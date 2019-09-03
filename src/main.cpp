#include <Arduino.h>
#include "./StateMachines/ActuatorStateMachine.h"
#include "./Pushbuttons/FsrPushbutton.h"

// buttons
ActuatorStateMachine actuatorStateMachine = ActuatorStateMachine();
FsrPushbutton testPushbutton = FsrPushbutton(A1, 5);

void setup()
{
	Serial.begin(115200);
	pinMode(A0, OUTPUT);


//	testPushbutton = FsrPushbutton(A1, 5);
	
	
	delay(5000);
	//ClearArray(valueArray, ARRAY_LENGTH);
	Serial.println("Delayed");
	// put your setup code here, to run once:
}

void loop()
{
	actuatorStateMachine.Process();
	
	// PressState bar = testPushbutton.IsPress();
	// if (bar == SINGLE_PRESS)
	// {
	// 	Serial.println("SINGLE PRESS: ");
	// }
	// if (bar == DOUBLE_PRESS)
	// 	Serial.println("DOUBLE PRESS: ");

	// testPushbutton.PollPresses();

	delay(100);
}

