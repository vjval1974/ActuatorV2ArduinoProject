// Physics model for the actuator simulator.
//
// Reads motor / solenoid pin states each tick, integrates an arm position,
// computes the FSR analog value the firmware will see on its next call to
// analogRead(A2), and ramps the vacuum and suction-cup-position sensors
// based on the firmware's commands. Also injects button-press patterns on
// A0/A1 when requested via QueueStartPress/QueueStopPress.

#ifndef SIM_PHYSICS_H
#define SIM_PHYSICS_H

#include <stdint.h>

namespace SimPhysics
{
    void Init();
    void Tick();

    void QueueStartPress();
    void QueueStopPress();
    void Reset();

    enum class MotorDirection : uint8_t { Stopped, UpFast, UpSlow, DownFast, DownSlow };

    struct Snapshot
    {
        float armPosition;
        float boardPosition;
        float fsrPercent;
        float vacuumLevel;     // 0..100 (%)
        float cupDeployment;   // 0..100 (% raised toward board)
        bool  hasVacuum;
        bool  cupRaised;
        bool  vacuumSolenoidActive;
        bool  cupSolenoidActive;
        MotorDirection motor;
    };

    Snapshot Read();
}

#endif
