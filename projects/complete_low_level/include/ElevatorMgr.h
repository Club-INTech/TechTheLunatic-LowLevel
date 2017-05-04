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

    bool isElevatorMoving() const;

    void getData();

    Position positionSetpoint;
    bool moveAbnormal;
    void setPWM(uint8_t pwm);

private:
    Elevator elevator;
    SensorMgr* sensorMgr;
    volatile int8_t elevatorPWM;
    volatile bool positionControlled;
    volatile bool moving;
    Position position;
    volatile bool isUp;
    volatile bool isDown;


    volatile uint32_t moveToOrderPing;
    volatile uint32_t delayToStop;
    uint32_t timeSinceMoveTo;
    uint32_t moveToPing;
};

#endif //ARM_ELEVATORMGR_H