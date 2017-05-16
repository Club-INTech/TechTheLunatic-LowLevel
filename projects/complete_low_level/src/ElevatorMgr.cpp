#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 10; //€[|0,10|]
    position = UP;
    positionSetpoint = DOWN;    //Pour toujours aller en bas au début
    positionControlled = true;
    moving = true;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp = sensorMgr->isContactor1engaged();
    isDown = sensorMgr->isContactor2engaged();
    delayToStop=430;
    timeSinceMoveTo=Millis();
    moveToPing=Millis();
}

void ElevatorMgr::enableAsserv(bool enable)
{
    positionControlled= enable;
}

/**
 * Fixe la position voulue pour l'ascenseur
 * @param positionToGo une ElevatorMgr::Position
 */
void ElevatorMgr::moveTo(Position positionToGo)
{
    enableAsserv(true);
    if (!moving)
    {
        moving = true;
    }
    positionSetpoint = positionToGo;
    moveAbnormal = false;
    moveToPing=Millis();
}

/**
 * Methode de contrôle en interruption
 */

//TODO: parfois pas de timeout alors que si (vers le bas)
void ElevatorMgr::control()
{
    if(positionControlled) //Si l'ascenseur est asservi
    {
        //On met à jour l'état des contacteurs
        isUp=!sensorMgr->isContactor1engaged();
        isDown=sensorMgr->isContactor2engaged();

        if(positionSetpoint==UP)
        {       //Si on a demandé à ce qu'on aille en haut
            elevator.setSens(Elevator::UP); //Le moteur va vers le haut
            if(!isUp && position!=UP) {
                if (Millis() - moveToPing < delayToStop) {
                    elevator.run(elevatorPWM);         //si il n'est pas arrivé , et si ça fait pas trop longtemps qu'on a envoyé l'ordre de bouger
                } else{
                    serial.printflnDebug("Timeout asc: going down");
                    position=UP;                      //Si non, on considère qu'on s'est bloqué et donc qu'on est en haut et qu'on doit redescendre
                    moveTo(DOWN);
                }
            }
            else if(isUp) {
                serial.printflnDebug("asc down, going up");
                position = UP;
                if (moving) {
                    stop();        //Si il est en haut et qu'il n'est pas arrété, il s'arrête
                    moveTo(DOWN);
                }
            }
        }
        else if(positionSetpoint==DOWN)
        {
            elevator.setSens(Elevator::DOWN);
            if(!isDown && position!=DOWN)
            {
                if (Millis() - moveToPing < delayToStop) {
                    elevator.run(elevatorPWM);
                }
                else{
                    serial.printflnDebug("Timeout asc: going up");
                    position=DOWN;
                    moveTo(DOWN);
                }
            }
            else if(isDown)
            {
                serial.printflnDebug("asc down");
                position=DOWN;
                    stop();
            }
        }
    }
    else{
        stop();

    }
}

/**
 * Arrête l'ascenseur à sa dernière position demandée
 */
void ElevatorMgr::stop()
{
    position = positionSetpoint;
    elevator.stop();
    moving=false;
    enableAsserv(false);
}

bool ElevatorMgr::isElevatorMoving() const
{
    return moving;
}

void ElevatorMgr::getData()
{
    if(position==UP)
    {
        serial.printflnDebug("pos: UP");
    }
    else if(position==DOWN)
    {
        serial.printflnDebug("pos: DOWN");
    }
    serial.printflnDebug("PWM: %d", elevatorPWM);
}

void ElevatorMgr::setPWM(uint8_t pwm){
    this->elevatorPWM=pwm;
}