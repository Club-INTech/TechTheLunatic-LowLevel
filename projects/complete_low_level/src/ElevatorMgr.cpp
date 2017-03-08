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
{   elevatorPWM = 110; //TODO why 110?
    elevatorPID.setTunings(10, 0, 0); //on initialise kp (proportionnel), ki (intégral) et kd (dérivé) du PID
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
    if ((setSens == UP)&&(Position=DOWN)) {
        positionSetpoint = (int32_t)(HEIGHT / TICKS_TO_MM);
    }
    else if ((setSens == DOWN)&&(Position=UP)){
        positionSetpoint = 0; //c'est la position initiale basse
    }
    moveAbnormal = false;
}



void ElevatorMgr::elevatorControl(){
    /*
	 * Comptage des ticks
	 * Cette codeuse est connect�e � un timer 16bit
	 * on subit donc un overflow/underflow de la valeur des ticks tous les 7 m�tres environ
	 * ceci est corrig� de mani�re � pouvoir parcourir des distances grandes sans devenir fou en chemin (^_^)
*/

/*
    //éviter overflow
    static int32_t lastRawTicks = 0;	//On garde en m�moire le nombre de ticks obtenu au pr�c�dent appel
    static int Overflow = 0;			//On garde en m�moire le nombre de fois que l'on a overflow (n�gatif pour les underflow)

    int32_t rawTicks = Counter::getMoteurValue();	//Nombre de ticks avant tout traitement

    if (lastRawTicks - rawTicks > 32768)		//D�tection d'un overflow
        Overflow++;
    else if(lastRawTicks - rawTicks < -32768)	//D�tection d'un underflow
        Overflow--;

    lastRawTicks = rawTicks;

    int32_t Ticks = rawTicks + Overflow*65535;	//On calcule le nombre r�el de ticks
*/

    //controle

    currentPosition = Counter::getMoteurValue();
    //serial.printflnDebug("%d: ah!", Counter::getMoteurValue());
    elevatorPID.compute();	// Actualise la valeur calculée par le PID
    //serial.printflnDebug("%d :elevatorPWM", elevatorPWM);
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
void ElevatorMgr::run()
{
    elevatorMotor.run(255);
}