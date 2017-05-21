#ifndef ARM_ELEVATORMGR_H
#define ARM_ELEVATORMGR_H

#include "../library/Singleton.hpp"
#include "Elevator.h"
#include "../library/delay.h"
#include <../library/Uart.hpp>
#include "SensorMgr.h"

extern Uart<1> serial;
class ElevatorMgr : public Singleton<ElevatorMgr>
{
public:
    enum Position{
        UP, DOWN
    };

    ElevatorMgr();
    void control();
    void moveTo(Position);
    void stop();

    void enableAsserv(bool);

    void getMovingState() const;

    void getData() const;

    void setPWM(uint8_t pwm);
    void resetTimeout();

private:
    Elevator elevator;
    SensorMgr* sensorMgr;

    volatile int8_t elevatorPWMUP;

    volatile bool positionControlled;
    volatile bool moving;
    volatile bool moveAbnormal;

    volatile bool isUp;
    volatile bool isDown;

    volatile Position position;
    volatile Position positionSetpoint;

    volatile uint32_t timeout;
    volatile uint32_t moveToPing;
    volatile uint8_t timeoutCount;
    int8_t elevatorPWMDOWN;
};

#endif //ARM_ELEVATORMGR_H