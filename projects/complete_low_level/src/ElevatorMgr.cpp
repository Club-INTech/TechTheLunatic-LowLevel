#include "../include/ElevatorMgr.h"


/**
 * Gestion de l'asserv' de l'ascenseur
 *
 * Ticks/tour de la codeuse: 500
 * Longueur à monter:~136mm
 * diamètre poulie:20mm
 *
 * @author melanie
 **/


ElevatorMgr::ElevatorMgr(): elevatorPID(&currentPosition, &elevatorPWM, &positionSetpoint) //position réelle, PWM, consigne de position
{   elevatorPWM = 0;
    elevatorPID.setTunings(0.006, 0, 0); //on initialise kp (proportionnel), ki (intégral) et kd (dérivé) du PID
    positionSetpoint = 0;

    Position = DOWN;

    positionControlled=true;
    elevatorMoving = false;
    delayStop = 120; //temps à l'arrêt avant de considérer un blocage
}
void ElevatorMgr::elevatorInit(){
    /*
            * Initialisation moteurs et encodeurs
    */
    elevatorMotor.initialize();
    Counter();
    enableAsserv(true);
    elevatorPID.setEpsilon(20);
    positionSetpoint=LowTicks;
}
void ElevatorMgr::enableAsserv(bool enable) {
    if (enable) {
        positionControlled=true;
    } else {
        positionControlled=false;
        elevatorStop();
    }
}



//orders
void ElevatorMgr::moveTo(Sens setSens) {

    if(!moving)
    {
        elevatorPID.resetErrors();
        moving = true;
    }
    if (setSens == UP) {
        positionSetpoint = HighTicks;   //Position qui bloque le module en haut
    }
    else if (setSens == DOWN){
        positionSetpoint = LowTicks;    //Position de prise de modules
    }
    moveAbnormal = false;
}



void ElevatorMgr::elevatorControl(){

    //Gestion du dépassement des ticks de codeuse(car timer en 16bits)
    static int32_t previousTick=0;
    static int overflow=0;
    int32_t rawCurrentPosition = Counter::getMoteurValue();
    if(previousTick-rawCurrentPosition > 32767)
        overflow++;
    else if(previousTick-rawCurrentPosition < -32768)
        overflow--;
    previousTick=rawCurrentPosition;
    currentPosition=rawCurrentPosition+overflow*65535;

    //contrôle
    elevatorPID.compute();	// Actualise la valeur calculée par le PID
    elevatorMotor.run(elevatorPWM);
}

void ElevatorMgr::elevatorStop(){
    positionSetpoint = currentPosition;
    elevatorMotor.run(elevatorPWM);
    elevatorMoving = false;
    elevatorPID.resetErrors();
    elevatorMotor.stop();
}

/*
* Getters/Setters des constantes d'asservissement en translation/rotation/vitesse
*/

void ElevatorMgr::getElevatorTunings(float &kp, float &ki, float &kd) const {
    kp = elevatorPID.getKp();
    ki = elevatorPID.getKi();
    kd = elevatorPID.getKd();
}
void ElevatorMgr::setElevatorTunings(float kp, float ki, float kd) {
    elevatorPID.setTunings(kp, ki, kd);
}

/*
 * Getters/Setters des variables de position haut niveau
 */

void ElevatorMgr::setDelayStop(uint32_t delayToStop)
{
    this->delayStop = delayToStop;
}

bool ElevatorMgr::elevatorIsMoving() const{
    return moving;
}

bool ElevatorMgr::elevatorMoveAbnormal() const{
    return moveAbnormal;
}

//fonctions de test codeuses/moteur
void ElevatorMgr::run(int i)
{
    elevatorMotor.run(i);
}

void ElevatorMgr::getData()
{
    serial.printflnDebug("pos: X -- tick: %d", currentPosition);
    serial.printflnDebug("Consigne: %d", positionSetpoint);
    serial.printflnDebug("PWM: %d", elevatorPWM);
    serial.printflnDebug("Erreur: %d", elevatorPID.getError());
    serial.printflnDebug("Input: %d", elevatorPID.getInput());
}