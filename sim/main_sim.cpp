// Native simulator entry point.
//
// Drives an ActuatorStateMachine instance at the same 20 ms cadence the
// firmware uses on a real Pro Micro. Each tick:
//   1. Polls stdin (non-blocking) for JSON command lines: {"start":true},
//      {"stop":true}, or {"reset":true}.
//   2. Calls physics.Tick() to integrate the arm and feed sensor pins.
//   3. Calls actuatorStateMachine.Process().
//   4. Emits a single JSON line on stdout describing the new state.
//
// stdout is line-buffered so the Python server can read state updates as
// they happen; the command channel runs in the opposite direction over
// stdin. Serial.print output is captured into a buffer and embedded in
// each state JSON object as a "log" field.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <fcntl.h>

#include "Arduino.h"
#include "physics.h"
#include "../src/StateMachines/ActuatorStateMachine.h"

static const char* MotorName(SimPhysics::MotorDirection d)
{
    switch (d)
    {
    case SimPhysics::MotorDirection::Stopped:  return "STOPPED";
    case SimPhysics::MotorDirection::UpFast:   return "UP_FAST";
    case SimPhysics::MotorDirection::UpSlow:   return "UP_SLOW";
    case SimPhysics::MotorDirection::DownFast: return "DOWN_FAST";
    case SimPhysics::MotorDirection::DownSlow: return "DOWN_SLOW";
    }
    return "STOPPED";
}

// We track the state machine's state externally by snooping the first byte
// of the ActuatorStateMachine object before/after Process(). That would be
// invasive; instead, expose a hook via inheritance.
class TrackedActuatorStateMachine : public ActuatorStateMachine
{
public:
    using ActuatorStateMachine::ActuatorStateMachine;
};

// Naive non-blocking stdin reader: drains all available bytes into a
// std::string, splitting on '\n'. Returns each line through the callback.
static std::string stdinAccum;
template <typename Fn>
static void DrainStdin(Fn handle)
{
    char buf[256];
    while (true)
    {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
        if (n <= 0) break;
        stdinAccum.append(buf, buf + n);
        size_t pos;
        while ((pos = stdinAccum.find('\n')) != std::string::npos)
        {
            handle(stdinAccum.substr(0, pos));
            stdinAccum.erase(0, pos + 1);
        }
    }
}

static bool jsonHasKeyTrue(const std::string& line, const char* key)
{
    // Tiny ad-hoc check, enough for {"start":true} / {"stop":true} / etc.
    std::string needle = "\"";
    needle += key;
    needle += "\"";
    auto k = line.find(needle);
    if (k == std::string::npos) return false;
    auto t = line.find("true", k);
    auto f = line.find("false", k);
    return t != std::string::npos && (f == std::string::npos || t < f);
}

// Hack: peek the ActuatorState out of the state machine. ActuatorState is
// a private member, but we'd rather not change firmware headers. Instead
// we infer the current high-level state from peripheral pins after each
// tick — every state in the machine has a unique signature of motor and
// solenoid pin patterns. Coarse but enough for visualisation; the
// authoritative state will be added via a small friend hook below.
//
// Simpler: re-read DEBUG serial logs which already announce transitions.
// We parse "Transitioning State from X to Y" out of the serial buffer.

static std::string currentStateName = "STOPPED";

static void ParseStateFromSerial(const std::string& s)
{
    size_t pos = 0;
    while (true)
    {
        size_t p = s.find("Transitioning State from ", pos);
        if (p == std::string::npos) break;
        size_t toAt = s.find(" to ", p);
        if (toAt == std::string::npos) break;
        size_t dotAt = s.find(". ", toAt);
        if (dotAt == std::string::npos) dotAt = s.find('\n', toAt);
        if (dotAt == std::string::npos) break;
        std::string newName = s.substr(toAt + 4, dotAt - (toAt + 4));
        currentStateName = newName;
        pos = dotAt;
    }
}

static void EmitState(const SimPhysics::Snapshot& s, const std::string& log)
{
    // Escape backslashes, quotes and newlines in the log string.
    std::string esc;
    esc.reserve(log.size());
    for (char c : log)
    {
        switch (c)
        {
        case '\\': esc += "\\\\"; break;
        case '"':  esc += "\\\""; break;
        case '\n': esc += "\\n";  break;
        case '\r': esc += "\\r";  break;
        case '\t': esc += "\\t";  break;
        default:
            if ((unsigned char)c < 0x20) {
                char b[8]; snprintf(b, sizeof(b), "\\u%04x", c);
                esc += b;
            } else {
                esc += c;
            }
        }
    }

    printf(
        "{\"state\":\"%s\","
        "\"motor\":\"%s\","
        "\"arm\":%.2f,"
        "\"board\":%.2f,"
        "\"fsr\":%.2f,"
        "\"vacuum\":%.2f,"
        "\"cup\":%.2f,"
        "\"hasVacuum\":%s,"
        "\"cupRaised\":%s,"
        "\"vacuumSolenoid\":%s,"
        "\"cupSolenoid\":%s,"
        "\"log\":\"%s\"}\n",
        currentStateName.c_str(),
        MotorName(s.motor),
        s.armPosition,
        s.boardPosition,
        s.fsrPercent,
        s.vacuumLevel,
        s.cupDeployment,
        s.hasVacuum ? "true" : "false",
        s.cupRaised ? "true" : "false",
        s.vacuumSolenoidActive ? "true" : "false",
        s.cupSolenoidActive ? "true" : "false",
        esc.c_str()
    );
    fflush(stdout);
}

int main()
{
    // Non-blocking stdin so we can poll for commands without stalling the
    // 20-ms tick loop.
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    setvbuf(stdout, nullptr, _IOLBF, 0);

    SimPhysics::Init();
    TrackedActuatorStateMachine sm;

    auto nextTick = std::chrono::steady_clock::now();
    const auto period = std::chrono::milliseconds(20);

    while (true)
    {
        // 1. Drain incoming commands.
        DrainStdin([](const std::string& line) {
            if (jsonHasKeyTrue(line, "start"))  SimPhysics::QueueStartPress();
            if (jsonHasKeyTrue(line, "stop"))   SimPhysics::QueueStopPress();
            if (jsonHasKeyTrue(line, "reset"))
            {
                SimPhysics::Reset();
                currentStateName = "STOPPED";
            }
        });

        // 2. Physics step (drives sensor pins).
        SimPhysics::Tick();

        // 3. Firmware step.
        sm.Process();

        // 4. Pull whatever Serial output the firmware wrote this tick.
        std::string log = SimPinState::serialBuffer;
        SimPinState::serialBuffer.clear();
        ParseStateFromSerial(log);

        // 5. Emit JSON state.
        EmitState(SimPhysics::Read(), log);

        // 6. Sleep until next tick.
        nextTick += period;
        std::this_thread::sleep_until(nextTick);
    }
    return 0;
}
