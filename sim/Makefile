# Native simulator build. Compiles the existing firmware sources against
# the Arduino.h / EEPROM.h shims in this directory and produces ./sim_bin,
# a standalone binary the Python server (serve.py) drives over stdio.

CXX      ?= g++
CXXFLAGS ?= -std=gnu++14 -O2 -Wall -Wextra -DSIM_BUILD -DDEBUG_MODE=1 -I. -I../src

SRC := \
	Arduino.cpp \
	physics.cpp \
	main_sim.cpp \
	../src/StateMachines/ActuatorStateMachine.cpp \
	../src/Motor/Motor.cpp \
	../src/Solenoids/VacuumSolenoid.cpp \
	../src/Solenoids/SuctionCupSolenoid.cpp \
	../src/Fsr/TouchSensor.cpp \
	../src/Pushbuttons/FsrPushbutton.cpp \
	../src/VacuumPressureSwitch/VacuumPressureSwitch.cpp \
	../src/Helpers/ArrayHelper.cpp \
	../src/Data/SavedData.cpp

sim_bin: $(SRC) Arduino.h EEPROM.h physics.h
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) -lpthread

clean:
	rm -f sim_bin

.PHONY: clean
