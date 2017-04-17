#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 0;
    position = DOWN;
    positionControlled=true;
    isElevatorMoving = false;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp=sensorMgr->isContactor1engaged();
    isDown=sensorMgr->isContactor2engaged();
}

void ElevatorMgr::enableAsserv(bool enable) {
    if (enable) {
        positionControlled=true;
    } else {
        positionControlled=false;
        stop();
    }
}

/**
 * Fixe la position voulue pour l'ascenseur
 * @param positionToGo une ElevatorMgr::Position
 */
void ElevatorMgr::moveTo(Position positionToGo) {
    if (!isElevatorMoving) {
        isElevatorMoving = true;
    }
    positionSetpoint = positionToGo;
    isMovementAbnormal = false;
}


/**
 * Methode de contrôle en interruption
 */
void ElevatorMgr::control(){
    if(positionControlled) //Si l'ascenseur est asservi
    {
        //On met à jour l'état des contacteurs
        isUp=sensorMgr->isContactor1engaged();
        isDown=sensorMgr->isContactor2engaged();

        if(positionSetpoint==UP){       //Si on a demandé à ce qu'on aille en haut
            elevator.setSens(Elevator::UP); //Le moteur va vers le haut
            if(!isUp){
                elevator.run();         //Tant qu'il n'est pas en haut, l'ascenseur monte
            }
            else{
                elevator.stop();        //Si il est en haut, il s'arrête
                position=UP;
            }
        }
        else if(positionSetpoint==DOWN){
            elevator.setSens(Elevator::DOWN);
            if(!isDown){
                elevator.run();
            }
            else{
                elevator.stop();
                position=DOWN;
            }
        }
    }
}

/**
 * Arrête l'ascenseur à sa dernière position demandée
 */
void ElevatorMgr::stop(){
    positionSetpoint = this->position;
    elevator.stop();
}

bool ElevatorMgr::elevatorIsMoving() const{
    return isElevatorMoving;
}

bool ElevatorMgr::elevatorMoveAbnormal() const{
    return isMovementAbnormal;
}

void ElevatorMgr::getData()
{
    if(position==UP)
    {
        serial.printflnDebug("pos: UP");
    }
    else if(position==DOWN){
        serial.printflnDebug("pos: DOWN");
    }
    serial.printflnDebug("PWM: %d", elevatorPWM);
}