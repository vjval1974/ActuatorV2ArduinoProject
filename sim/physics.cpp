#include "physics.h"
#include "Arduino.h"

// Pro Micro pin assignments — mirror the firmware's pin map.
static constexpr uint8_t kMotorFwPin       = 2;
static constexpr uint8_t kMotorBwPin       = 3;
static constexpr uint8_t kMotorStopPin     = 4;
static constexpr uint8_t kMotorSpeed2Pin   = 5;
static constexpr uint8_t kMotorFaultPin    = 6;
static constexpr uint8_t kSuctionCupSolPin = 9;
static constexpr uint8_t kCupPositionPin   = 14;
static constexpr uint8_t kVacuumSwitchPin  = 10;
static constexpr uint8_t kVacuumSolPin     = 8;
static constexpr uint8_t kStartButtonPin   = A0;
static constexpr uint8_t kStopButtonPin    = A1;
static constexpr uint8_t kFsrPin           = A2;

// Tuning constants (all in arbitrary "physics units"; tick = 20 ms).
static constexpr float kBoardPosition          = 50.0f;
static constexpr float kArmMin                 = 0.0f;
static constexpr float kArmMax                 = 65.0f;
static constexpr float kFastSpeed              = 1.5f;    // units/tick
static constexpr float kSlowSpeed              = 0.15f;   // units/tick
static constexpr float kFsrAnalogPerUnit       = 90.0f;   // raw counts per unit of compression
static constexpr float kCupDeployRate          = 6.0f;    // % per tick (~330 ms full travel)
static constexpr float kVacuumBuildRate        = 4.0f;    // % per tick when sealed
static constexpr float kVacuumDecayRate        = 6.0f;    // % per tick when not sealed
static constexpr float kVacuumThresholdPercent = 60.0f;
static constexpr float kCupRaisedThresholdPct  = 90.0f;

// Button-press pattern (analog values fed to A0/A1 over successive ticks).
// FsrPushbutton::IsRelease() looks for valueArray[0]==0 && valueArray[1]>0
// && any of the first 5 elements > 10. After IsRelease() fires, IsPress()
// returns SINGLE_PRESS once _oneReleaseCount exceeds _numSpacesForTimeout
// (5). So we drive the pin high for several ticks, then low.
static constexpr int kButtonHighTicks = 4;
static constexpr int kButtonHighValue = 200;

namespace
{
    float armPos       = kArmMin;
    float cupDeploy    = 0.0f;
    float vacuumLevel  = 0.0f;
    int   startTicksLeft = 0;
    int   stopTicksLeft  = 0;

    SimPhysics::MotorDirection ReadMotorDirection()
    {
        // The firmware's MotorController::MotorDrive() leaves stopPin HIGH
        // for every command and uses (fwPin, bwPin) to select direction
        // and speed2Pin to select speed.
        uint8_t fw     = SimPinState::digitalLevels[kMotorFwPin];
        uint8_t bw     = SimPinState::digitalLevels[kMotorBwPin];
        uint8_t speed2 = SimPinState::digitalLevels[kMotorSpeed2Pin];

        // Both HIGH = stopped.
        if (fw == HIGH && bw == HIGH) return SimPhysics::MotorDirection::Stopped;
        // fw LOW, bw HIGH = up.
        if (fw == LOW && bw == HIGH)
            return speed2 == HIGH ? SimPhysics::MotorDirection::UpFast
                                  : SimPhysics::MotorDirection::UpSlow;
        // fw HIGH, bw LOW = down.
        if (fw == HIGH && bw == LOW)
            return speed2 == HIGH ? SimPhysics::MotorDirection::DownFast
                                  : SimPhysics::MotorDirection::DownSlow;
        return SimPhysics::MotorDirection::Stopped;
    }

    bool VacuumSolenoidActive()
    {
        // Active-LOW: firmware does digitalWrite(LOW) on ACTIVATE.
        return SimPinState::digitalLevels[kVacuumSolPin] == LOW;
    }

    bool CupSolenoidActive()
    {
        // Active-HIGH: firmware does digitalWrite(HIGH) on ACTIVATE.
        return SimPinState::digitalLevels[kSuctionCupSolPin] == HIGH;
    }
}

void SimPhysics::Init()
{
    armPos = kArmMin;
    cupDeploy = 0.0f;
    vacuumLevel = 0.0f;
    startTicksLeft = 0;
    stopTicksLeft = 0;
    SimPinState::analogLevels[kStartButtonPin] = 0;
    SimPinState::analogLevels[kStopButtonPin]  = 0;
    SimPinState::analogLevels[kFsrPin]         = 0;
    SimPinState::digitalLevels[kCupPositionPin]  = HIGH; // pullup default
    SimPinState::digitalLevels[kVacuumSwitchPin] = LOW;  // no vacuum
}

void SimPhysics::Reset() { Init(); }

void SimPhysics::QueueStartPress() { startTicksLeft = kButtonHighTicks; }
void SimPhysics::QueueStopPress()  { stopTicksLeft  = kButtonHighTicks; }

void SimPhysics::Tick()
{
    // 1) Drive analog values for the start/stop FsrPushbuttons.
    SimPinState::analogLevels[kStartButtonPin] =
        startTicksLeft > 0 ? kButtonHighValue : 0;
    SimPinState::analogLevels[kStopButtonPin] =
        stopTicksLeft > 0 ? kButtonHighValue : 0;
    if (startTicksLeft > 0) --startTicksLeft;
    if (stopTicksLeft  > 0) --stopTicksLeft;

    // 2) Integrate arm position from the motor command.
    MotorDirection dir = ReadMotorDirection();
    float delta = 0.0f;
    switch (dir)
    {
    case MotorDirection::UpFast:   delta = +kFastSpeed; break;
    case MotorDirection::UpSlow:   delta = +kSlowSpeed; break;
    case MotorDirection::DownFast: delta = -kFastSpeed; break;
    case MotorDirection::DownSlow: delta = -kSlowSpeed; break;
    case MotorDirection::Stopped:  delta = 0.0f;        break;
    }
    armPos += delta;
    if (armPos < kArmMin) armPos = kArmMin;
    if (armPos > kArmMax) armPos = kArmMax;

    // 3) FSR raw value: zero below the board, proportional to compression
    //    above it. Map back to the 0-1023 ADC range so the firmware's
    //    GetFsrPct() (which does map(x, 0, 1023, 0, 100)) produces sensible
    //    percentages.
    float compression = armPos - kBoardPosition;
    if (compression < 0) compression = 0;
    int fsrRaw = (int)(compression * kFsrAnalogPerUnit);
    if (fsrRaw > 1023) fsrRaw = 1023;
    SimPinState::analogLevels[kFsrPin] = fsrRaw;

    // 4) Cup deployment ramps toward whichever the solenoid is commanding.
    bool cupOn = CupSolenoidActive();
    if (cupOn)  cupDeploy += kCupDeployRate;
    else        cupDeploy -= kCupDeployRate;
    if (cupDeploy < 0)    cupDeploy = 0;
    if (cupDeploy > 100)  cupDeploy = 100;
    bool cupRaised = cupDeploy >= kCupRaisedThresholdPct;
    // Cup position switch: active-LOW via pullup (HIGH = raised in code:
    // SuctionCup::GetPosition returns SUCTION_CUP_RAISED when pin reads HIGH).
    SimPinState::digitalLevels[kCupPositionPin] = cupRaised ? HIGH : LOW;

    // 5) Vacuum builds only when the solenoid is active AND the cup is
    //    sealed against the board (raised AND board pressed).
    bool sealed = VacuumSolenoidActive() && cupRaised && (armPos >= kBoardPosition - 1.0f);
    if (sealed)  vacuumLevel += kVacuumBuildRate;
    else         vacuumLevel -= kVacuumDecayRate;
    if (vacuumLevel < 0)    vacuumLevel = 0;
    if (vacuumLevel > 100)  vacuumLevel = 100;
    bool hasVac = vacuumLevel >= kVacuumThresholdPercent;
    SimPinState::digitalLevels[kVacuumSwitchPin] = hasVac ? HIGH : LOW;
}

SimPhysics::Snapshot SimPhysics::Read()
{
    Snapshot s;
    s.armPosition          = armPos;
    s.boardPosition        = kBoardPosition;
    s.fsrPercent           = (SimPinState::analogLevels[kFsrPin] * 100.0f) / 1023.0f;
    s.vacuumLevel          = vacuumLevel;
    s.cupDeployment        = cupDeploy;
    s.hasVacuum            = SimPinState::digitalLevels[kVacuumSwitchPin] == HIGH;
    s.cupRaised            = cupDeploy >= kCupRaisedThresholdPct;
    s.vacuumSolenoidActive = VacuumSolenoidActive();
    s.cupSolenoidActive    = CupSolenoidActive();
    s.motor                = ReadMotorDirection();
    return s;
}
