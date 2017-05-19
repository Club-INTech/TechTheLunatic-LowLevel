#include "../include/ElevatorMgr.h"

/**
 * Gestion de la position de l'ascenseur
 * @author melanie, ug
 **/


ElevatorMgr::ElevatorMgr()
{
    //Initialise tous les paramètres
    elevatorPWM = 8; //€[|0,10|]
    position = UP;
    positionSetpoint = DOWN;    //Pour toujours aller en bas au début
    positionControlled = true;
    moving = true;
    elevator.initialize();
    enableAsserv(true);
    sensorMgr = &SensorMgr::Instance();
    isUp = sensorMgr->isContactor1engaged();
    isDown = sensorMgr->isContactor2engaged();
    timeout=1000;
    moveToPing=Millis();    //Un timeout moveTo arrête l'ascenseur et le ramène a sa position précédente si il n'arrive pas à destination
    timeoutCount=0;
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

void ElevatorMgr::control()
{
    if(positionControlled && timeoutCount<2) //Si l'ascenseur est asservi
    {
        //On met à jour l'état des contacteurs
        isUp=sensorMgr->isContactor1engaged();
        isDown=sensorMgr->isContactor2engaged();

        if(positionSetpoint==UP)
        {       //Si on a demandé à ce qu'on aille en haut
            if(!isUp && position!=UP)
            {
                if (Millis() - moveToPing < timeout)
                {
                    elevator.setSens(Elevator::UP); //Le moteur va vers le haut
                    elevator.run(elevatorPWM);         //si il n'est pas arrivé , et si ça fait pas trop longtemps qu'on a envoyé l'ordre de bouger
                }

                else
                {
                    stop();
                    timeoutCount++;
                    moveAbnormal=true;
                    //Si non, on considère qu'on s'est bloqué ouqu'on est en haut, donc qu'on doit redescendre
                    moveTo(DOWN);
                }
            }
            else if(isUp)
            {
                position = UP;
                if (moving)
                {
                    stop();        //Si il est en haut et qu'il n'est pas arrété, il s'arrête
                    moveTo(DOWN);
                }
            }
        }
        else if(positionSetpoint==DOWN)
        {
            if(!isDown && position!=DOWN)
            {
                if (Millis() - moveToPing < timeout)
                {
                    elevator.setSens(Elevator::DOWN);
                    elevator.run(elevatorPWM);
                }
                else
                {
                    stop();
                    timeoutCount++;
                    moveAbnormal=true;
                    moveTo(UP);
                }
            }
            else if(isDown)
            {
                position=DOWN;
                stop();
            }
        }
    }
    else
    {
        stop();
    }
}

/**
 * Arrête l'ascenseur
 */
void ElevatorMgr::stop()
{
    position = positionSetpoint;
    elevator.stop();
    moving=false;
    enableAsserv(false);
}

void ElevatorMgr::getMovingState() const
{
    serial.printflnDebug("%d", moving);
    serial.printflnDebug("%d", moveAbnormal);
}

void ElevatorMgr::getData() const
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

void ElevatorMgr::resetTimeout(){
    timeoutCount=0;
}