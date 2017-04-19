#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 5; //€[|0,10|]
    position = DOWN;
    positionSetpoint = position;
    positionControlled = true;
    moving = false;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp = sensorMgr->isContactor1engaged();
    isDown = sensorMgr->isContactor2engaged();
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
    if (!moving)
    {
        moving = true;
    }
    positionSetpoint = positionToGo;
    moveAbnormal = false;
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
            if(!isUp && !moving)
            {
                elevator.run(elevatorPWM);         //si il n'est pas arrivé , et ne bouge pas, il démarre
            }
            else if(isUp && moving)
            {
                position=UP;
                stop();        //Si il est en haut et qu'il n'est pas arrété, il s'arrête
            }
        }
        else if(positionSetpoint==DOWN)
        {
            elevator.setSens(Elevator::DOWN);
            if(!isDown && !moving)
            {
                elevator.run(elevatorPWM);
            }
            else if(isDown && moving)
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