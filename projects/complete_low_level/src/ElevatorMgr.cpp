#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 110; //€[|0,255|]
    position = UP;
    positionSetpoint = DOWN;    //Pour toujours aller en bas au début
    positionControlled = true;
    moving = false;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp = sensorMgr->isContactor1engaged();
    isDown = sensorMgr->isContactor2engaged();

    delayToStop=1500;
    moveToOrderPing=Millis();
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
    moveToOrderPing=Millis();
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
                elevator.run(elevatorPWM);         //si il n'est pas arrivé , et si ça fait pas trop longtemps qu'on a envoyé l'ordre de bouger
            }
            else if(isUp)
            {
                position=UP;
                stop();
                moveTo(DOWN);
            }
        }
        else if(positionSetpoint==DOWN)
        {
            elevator.setSens(Elevator::DOWN);
            if(!isDown && position!=DOWN)
            {
                elevator.run(elevatorPWM);
            }
            else if(isDown)
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

void ElevatorMgr::manageStop()
{
    static uint32_t time = Millis();
    if(moving && time-moveToOrderPing>delayToStop){
        enableAsserv(false);
        position=positionSetpoint;
        stop();
    }
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