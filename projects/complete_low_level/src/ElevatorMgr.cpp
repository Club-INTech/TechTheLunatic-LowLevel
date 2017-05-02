#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 122; //€[|0,255|]
    position = UP;
    positionSetpoint = DOWN;    //Pour toujours aller en bas au début
    positionControlled = true;
    moving = false;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp = sensorMgr->isContactor1engaged();
    isDown = sensorMgr->isContactor2engaged();
    delayToStop=500;
    timeSinceMoveTo=Millis();
    moveToPing=Millis();
}

void ElevatorMgr::enableAsserv(bool enable)
{
    if (enable)
    {
        positionControlled=true;
    } else
    {
        positionControlled=false;
        stop();
    }
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
void ElevatorMgr::control()
{    //TODO: implémenter un délai de blocage de l'ascenseur
    if(positionControlled) //Si l'ascenseur est asservi
    {
        //On met à jour l'état des contacteurs
        isUp=sensorMgr->isContactor1engaged();
        isDown=sensorMgr->isContactor2engaged();

        if(positionSetpoint==UP)
        {       //Si on a demandé à ce qu'on aille en haut
            elevator.setSens(Elevator::UP); //Le moteur va vers le haut
            if(!isUp && position!=UP) {
                if (Millis() - moveToPing < delayToStop) {
                    elevator.run(elevatorPWM);         //si il n'est pas arrivé , et si ça fait pas trop longtemps qu'on a envoyé l'ordre de bouger
                }
            }
            else if(isUp || (Millis()-moveToPing>delayToStop))
            {
                position=UP;
                stop();        //Si il est en haut et qu'il n'est pas arrété, il s'arrête
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
            }
            else if(isDown || (Millis()-moveToPing>delayToStop))
            {
                position=DOWN;
                stop();
            }
        }
    }
}

/**
 * Arrête l'ascenseur à sa dernière position demandée
 */
void ElevatorMgr::stop()
{
    positionSetpoint = this->position;
    elevator.stop();
    moving=false;
    enableAsserv(false);
}

bool ElevatorMgr::isElevatorMoving() const
{
    return moving;
}

bool ElevatorMgr::elevatorMoveAbnormal() const
{
    return moveAbnormal;
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

void ElevatorMgr::setPWM(int pwm){
    this->elevatorPWM=pwm;
}