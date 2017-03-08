//
// Created by tic-tac on 14/01/17.
//

/**
 *
 */

#ifndef ARM_ELEVATORMGR_H
#define ARM_ELEVATORMGR_H

#define PI 3.14159265
#define rayon 10
#define circonf (2*PI*rayon)


#define TICKS_TO_MM 0.2088			// unit� : mm/ticks
//TODO : à trouver empiriquement

#define HEIGHT 135 //unit : mm


#include "../library/Singleton.hpp"
#include "Elevator.h"
#include "../library/pid.hpp"
#include <math.h>
#include "../library/delay.h"
#include "../cmsis_lib/include/misc.h"
#include "Counter.h"
#include <../library/Uart.hpp>
#include "../library/average.hpp"
#include <../library/utils.h>

extern Uart<1> serial;
class ElevatorMgr : public Singleton<ElevatorMgr>
{
public:
    enum Sens{
        UP, DOWN
    };
private:
    Elevator elevatorMotor;
    PID elevatorPID;
    volatile int32_t positionSetpoint;	    // ticks            //consigne position
    volatile int32_t currentPosition;		// ticks            //position réelle
    volatile int32_t elevatorPWM;		// ticks/seconde        //PWM

    volatile bool positionControlled;
    volatile bool elevatorMoving;
    unsigned int delayStop;  //En ms
    volatile bool moving;
    volatile bool moveAbnormal;
    Sens Position;


public:
    ElevatorMgr();
    void elevatorInit();
    void elevatorControl();
    void moveTo(Sens);
    void elevatorStop();

    void enableAsserv(bool);
    void setDelayStop(uint32_t);

    void getElevatorTunings(float &,float &,float &) const;
    void setElevatorTunings(float, float, float);

    bool elevatorIsMoving() const;
    bool elevatorMoveAbnormal() const;

    void run();

    void getData();
};


#endif //ARM_ELEVATORMGR_H
