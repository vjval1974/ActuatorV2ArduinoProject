#include <Arduino.h>
#include "./StateMachines/DriveUpStateMachine.h"
#include "./Pushbuttons/FsrPushbutton.h"

// buttons
FsrPushbutton upPushbutton = FsrPushbutton(A1, 5);
FsrPushbutton downPushbutton = FsrPushbutton(A2, 5);

void setup()
{
	Serial.begin(115200);
	pinMode(A0, OUTPUT);


	upPushbutton = FsrPushbutton(A1, 5);
	downPushbutton = FsrPushbutton(A2, 5);
	
	delay(5000);
	//ClearArray(valueArray, ARRAY_LENGTH);
	Serial.print("Delayed");
	// put your setup code here, to run once:
}

void loop()
{
	//DriveUpStateMachine();
	
	PressState bar = upPushbutton.IsPress();
	if (bar == SINGLE_PRESS)
	{
		Serial.println("SINGLE PRESS: ");
	}
	if (bar == DOUBLE_PRESS)
		Serial.println("DOUBLE PRESS: ");

	upPushbutton.PollPresses();

	delay(100);
}

