#include "../include/MotionControlSystem.h"


MotionControlSystem::MotionControlSystem(): leftMotor(Side::LEFT), rightMotor(Side::RIGHT),
                                            rightSpeedPID(&currentRightSpeed, &rightPWM, &rightSpeedSetpoint),
                                            leftSpeedPID(&currentLeftSpeed, &leftPWM, &leftSpeedSetpoint),
                                            translationPID(&currentDistance, &translationSpeed, &translationSetpoint),
                                            rotationPID(&currentAngle, &rotationSpeed, &rotationSetpoint),
                                            averageLeftSpeed(), averageRightSpeed() {
    translationControlled = true;
    rotationControlled = true;
    leftSpeedControlled = true;
    rightSpeedControlled = true;
    originalAngle = 0.0;
    rotationSetpoint = 0;
    translationSetpoint = 0;
    leftSpeedSetpoint = 0;
    rightSpeedSetpoint = 0;
    x = 0;
    y = 0;
    moving = false;
    moveAbnormal = false;
    curveMovement = false;
    forcedMovement = false;
    direction = NONE;

    leftCurveRatio = 1.0;
    rightCurveRatio = 1.0;

    leftSpeedPID.setOutputLimits(-10, 10);
    rightSpeedPID.setOutputLimits(-10, 10);

    maxSpeed = 3000; // Vitesse maximum, des moteurs (avec une marge au cas o� on s'amuse � faire forcer un peu la bestiole).
    maxSpeedTranslation = 2000; // Consigne max envoy�e au PID
    maxSpeedRotation = 1400;


    maxAccelAv = 8;
    maxDecelAv = 7;
    maxAccelAr = 12;
    maxDecelAr = 3;

    // maxjerk = 1; // Valeur de jerk maxi(secousse d'acc�l�ration)

    delayToStop = 100; // temps � l'arr�t avant de consid�rer un blocage
    delayToStopCurve = 500; // pareil en courbe
    toleranceTranslation = 20;
    toleranceRotation = 40;
    toleranceSpeed = 40;
    toleranceSpeedEstablished = 50; // Doit �tre la plus petite possible, sans bloquer les trajectoires courbes 50
    delayToEstablish = 100;

    toleranceCurveRatio = 0.9;
    toleranceDifferentielle = 500; // Pour les trajectoires "normales", v�rifie que les roues ne font pas nawak chacunes de leur cot�.

    translationPID.setTunings(12, 0, 200);
    rotationPID.setTunings(18, 0, 200);
    leftSpeedPID.setTunings(0.011, 0, 0.005); // ki 0.00001
    rightSpeedPID.setTunings(0.011, 0, 0.005);

    distanceTest = 200;

    maxAcceleration=maxAccelAv;
    maxDeceleration=maxDecelAv;
}

void MotionControlSystem::init() {
/**
 * Initialisation moteurs et encodeurs
 */
    Motor::initPWM();
    Counter();

/**
 * Initialisation de la boucle d'asservissement (TIMER 4)
 */
    NVIC_InitTypeDef NVIC_InitStructure;
    //Configuration et activation de l'interruption
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //Activation de l'horloge du TIMER 4
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    //Configuration du timer
    //TIM4CLK = HCLK / 2 = SystemCoreClock /2 = 168MHz/2 = 84MHz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1; // 84 MHz Clock down to 1 MHz
    TIM_TimeBaseStructure.TIM_Period = 500 - 1; // 1 MHz down to 2 KHz : fr�quence d'asservissement de 2kHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    //Active l'asservissement
    enable(true);
}

void MotionControlSystem::enable(bool enable) {
    if (enable) {
        TIM_Cmd(TIM4, ENABLE); //Active la boucle d'asservissement
    } else {
        TIM_Cmd(TIM4, DISABLE); //D�sactive la boucle d'asservissement
        stop();
    }
}

void MotionControlSystem::enableTranslationControl(bool enabled) {
    translationControlled = enabled;
}
void MotionControlSystem::enableRotationControl(bool enabled) {
    rotationControlled = enabled;
}

void MotionControlSystem::enableSpeedControl(bool enabled){
    leftSpeedControlled = enabled;
    rightSpeedControlled = enabled;
}


void MotionControlSystem::setRawPositiveTranslationSpeed(){
    translationSpeed = maxSpeedTranslation;
}

void MotionControlSystem::setRawPositiveRotationSpeed(){
    rotationSpeed = maxSpeedRotation;
}

void MotionControlSystem::setRawNegativeTranslationSpeed(){
    translationSpeed = -maxSpeedTranslation;
}

void MotionControlSystem::setRawNegativeRotationSpeed(){
    rotationSpeed = -maxSpeedRotation;
}

void MotionControlSystem::setRawNullSpeed(){
    rotationSpeed = 0;
    translationSpeed =0;
}

void MotionControlSystem::control()
{

    // Pour le calcul de la vitesse instantan�e :
    static int32_t previousLeftTicks = 0;
    static int32_t previousRightTicks = 0;

    // Pour le calcul de l'acc�l�ration intantan�e :
    static int32_t previousLeftSpeedSetpoint = 0;
    static int32_t previousRightSpeedSetpoint = 0;

    /*
    // Pour le calcul du jerk :
    static int32_t previousLeftAcceleration = 0;
    static int32_t previousRightAcceleration = 0;
    */


    /*
     * Comptage des ticks de la roue droite
     * Cette codeuse est connect�e � un timer 16bit
     * on subit donc un overflow/underflow de la valeur des ticks tous les 7 m�tres environ
     * ceci est corrig� de mani�re � pouvoir parcourir des distances grandes sans devenir fou en chemin (^_^)
     */
    static int32_t lastRawRightTicks = 0;	//On garde en m�moire le nombre de ticks obtenu au pr�c�dent appel
    static int rightOverflow = 0;			//On garde en m�moire le nombre de fois que l'on a overflow (n�gatif pour les underflow)

    int32_t rawRightTicks = Counter::getRightValue();	//Nombre de ticks avant tout traitement

    if (lastRawRightTicks - rawRightTicks > 32768)		//D�tection d'un overflow
        rightOverflow++;
    else if(lastRawRightTicks - rawRightTicks < -32768)	//D�tection d'un underflow
        rightOverflow--;

    lastRawRightTicks = rawRightTicks;

    rightTicks = rawRightTicks + rightOverflow*65535;	//On calcule le nombre r�el de ticks

    /*
     * Comptage des ticks de la roue gauche
     * ici on est sur un timer 32bit, pas de probl�me d'overflow sauf si on tente de parcourir plus de 446km...
     */
    leftTicks = Counter::getLeftValue();


    currentLeftSpeed = (leftTicks - previousLeftTicks)*2000; // (nb-de-tick-passés)*(freq_asserv) (ticks/sec)
    currentRightSpeed = (rightTicks - previousRightTicks)*2000;

    previousLeftTicks = leftTicks;
    previousRightTicks = rightTicks;

    averageLeftSpeed.add(currentLeftSpeed);
    averageRightSpeed.add(currentRightSpeed);

    averageLeftDerivativeError.add(ABS(leftSpeedPID.getDerivativeError()));		// Mise à jour des moyennes de dérivées de l'erreur (pour les blocages)
    averageRightDerivativeError.add(ABS(rightSpeedPID.getDerivativeError()));

    currentLeftSpeed = averageLeftSpeed.value(); // On utilise pour l'asserv la valeur moyenne des dernieres current Speed
    currentRightSpeed = averageRightSpeed.value(); // sinon le robot il fait nawak.


    currentDistance = (leftTicks + rightTicks) / 2;
    currentAngle = ((rightTicks - currentDistance)*RAYON_COD_GAUCHE/RAYON_COD_DROITE - (leftTicks - currentDistance)) / 2;


    if(translationControlled)
        translationPID.compute();	// Actualise la valeur de 'translationSpeed'
    if(rotationControlled)
        rotationPID.compute();		// Actualise la valeur de 'rotationSpeed'

    // Limitation de la consigne de vitesse en translation
    if(translationSpeed > maxSpeedTranslation)
        translationSpeed = maxSpeedTranslation;
    else if(translationSpeed < -maxSpeedTranslation)
        translationSpeed = -maxSpeedTranslation;

    // Limitation de la consigne de vitesse en rotation
    if(rotationSpeed > maxSpeedRotation)
        rotationSpeed = maxSpeedRotation;
    else if(rotationSpeed < -maxSpeedRotation)
        rotationSpeed = -maxSpeedRotation;

    leftSpeedSetpoint = (int32_t) (translationSpeed * leftCurveRatio - rotationSpeed);
    rightSpeedSetpoint = (int32_t) (translationSpeed * rightCurveRatio + rotationSpeed);

    // Limitation de la vitesse
    if(leftSpeedSetpoint > maxSpeed)
        leftSpeedSetpoint = maxSpeed;
    else if(leftSpeedSetpoint < -maxSpeed)
        leftSpeedSetpoint = -maxSpeed;
    if(rightSpeedSetpoint > maxSpeed)
        rightSpeedSetpoint = maxSpeed;
    else if(rightSpeedSetpoint < -maxSpeed)
        rightSpeedSetpoint = -maxSpeed;

    // Limitation de l'accélération du moteur gauche (permet de règler la pente du trapèze de vitesse)
    if(leftSpeedSetpoint - previousLeftSpeedSetpoint > maxAcceleration)
    {
        leftSpeedSetpoint = (int32_t) (previousLeftSpeedSetpoint + maxAcceleration * leftCurveRatio);
    }

        // Limitation de la décélération du moteur gauche
    else if (previousLeftSpeedSetpoint - leftSpeedSetpoint > maxDeceleration) {
        leftSpeedSetpoint = (int32_t) (previousLeftSpeedSetpoint - maxDeceleration * leftCurveRatio);
    }


    // Limitation de l'acc�l�ration du moteur droit
    if (rightSpeedSetpoint - previousRightSpeedSetpoint > maxAcceleration) {
        rightSpeedSetpoint = (int32_t) (previousRightSpeedSetpoint + maxAcceleration * rightCurveRatio);
    }

        // Limitation de la décélération du moteur droit
    else if (previousRightSpeedSetpoint - rightSpeedSetpoint > maxDeceleration) {
        rightSpeedSetpoint = (int32_t) (previousRightSpeedSetpoint - maxDeceleration * rightCurveRatio);
    }


    previousLeftSpeedSetpoint = leftSpeedSetpoint;			// Mise à jour des consignes de vitesse
    previousRightSpeedSetpoint = rightSpeedSetpoint;



    if(leftSpeedControlled)
        leftSpeedPID.compute();		// Actualise la valeur de 'leftPWM'
    else
        leftPWM = 0;
    if(rightSpeedControlled)
        rightSpeedPID.compute();	// Actualise la valeur de 'rightPWM'
    else
        rightPWM = 0;

    leftMotor.run(leftPWM);
    rightMotor.run(rightPWM);
}


bool MotionControlSystem::isPhysicallyStopped() {
    return ((translationPID.getDerivativeError() == 0) && (rotationPID.getDerivativeError() == 0)) || (ABS(ABS(leftSpeedPID.getError())-ABS(rightSpeedPID.getError()))>toleranceDifferentielle);
}


bool MotionControlSystem::isLeftWheelSpeedAbnormal() {
    return (ABS(leftSpeedPID.getError())>toleranceSpeedEstablished);
}

bool MotionControlSystem::isRightWheelSpeedAbnormal() {
    return (ABS(rightSpeedPID.getError())>toleranceSpeedEstablished);
}

void MotionControlSystem::enableForcedMovement(){
    forcedMovement=true;
}

void MotionControlSystem::disableForcedMovement(){
    forcedMovement=false;
}


void MotionControlSystem::manageStop()
{
    static uint32_t time = 0;
    static uint32_t time2 = 0;
    static uint32_t time3 = 0;
    static int32_t timeToEstablish = 0;
    static bool isSpeedEstablished = false;

    if (moving&&
        averageLeftDerivativeError.value()<toleranceSpeedEstablished &&
        averageRightDerivativeError.value()<toleranceSpeedEstablished &&

        leftSpeedPID.getError()<toleranceSpeed &&
        rightSpeedPID.getError()<toleranceSpeed &&

        !forcedMovement){

        if(timeToEstablish==0){
            timeToEstablish=Millis();
        }

        else if((timeToEstablish > delayToEstablish) && !isSpeedEstablished){
            isSpeedEstablished = true;

        }
    }

/*
	else if(moving && !forcedMovement){ //�cart � la consigne trop grand
		serial.printfln("Bien je commence � voir que je suis bloque");
			if(timeNotEstablished==0){
				timeNotEstablished=Millis();
				serial.printfln("not established vaut 0");
			}
			else if(timeNotEstablished > maxTimeNotEstablished){
				stop();
				moveAbnormal = true;
				serial.printfln("la je me bloque");
			}
		}
*/


//-----------//

    if (isPhysicallyStopped() && moving && !curveMovement && !forcedMovement) // Pour un blocage classique
    {
        if (time == 0)
        { //D�but du timer
            time = Millis();
        }
        else
        {
            if ((Millis() - time) >= delayToStop)
            { //Si arr�t� plus de 'delayToStop' ms
                { //Stop� pour cause de fin de mouvement
                    serial.printflnDebug("avant affectation moveAbnormal=%d", moveAbnormal);
                    serial.printflnDebug("transl error: %d, rot error: %d", translationPID.getError(), rotationPID.getError());
                    moveAbnormal=!(ABS((translationPID.getError()) <= toleranceTranslation) && ABS(rotationPID.getError()) <= toleranceRotation);
                    serial.printflnDebug("moveAbnormal=%d", moveAbnormal);
                    stop();
                }
            }
        }
    }

    else if(moving && !isSpeedEstablished && !forcedMovement && curveMovement){ // V�rifie que le ratio reste bon pdt les traj courbes

        if (leftCurveRatio<rightCurveRatio && averageRightSpeed.value() !=0 && rightCurveRatio!=0){ // si on tourne a gauche
            if (ABS((averageLeftSpeed.value()/averageRightSpeed.value())-(leftCurveRatio/rightCurveRatio))>toleranceCurveRatio){
                stop();
                moveAbnormal = true;
            }
        }
        else if(rightCurveRatio<leftCurveRatio && averageLeftSpeed.value()!=0 && leftCurveRatio!=0){ //si on tourne � droite
            if (ABS((averageRightSpeed.value()/averageLeftSpeed.value())-(rightCurveRatio/leftCurveRatio))>toleranceCurveRatio){
                stop();
                moveAbnormal = true;
            }
        }
    }


    else if ((isLeftWheelSpeedAbnormal() || isRightWheelSpeedAbnormal()) && curveMovement && !forcedMovement) // Sert a v�rifier que les consignes de vitesse sont bien respect�es (blocage pour les trajectoires courbes)
    {
        if (time2 == 0)
        { //D�but du timer
            time2 = Millis();
        }
        else
        {
            if (ABS(translationPID.getError()) <= toleranceTranslation && ABS(rotationPID.getError()) <= toleranceRotation)
            { //Stopp� pour cause de fin de mouvement
                stop();
                isSpeedEstablished = false;
                moveAbnormal = false;
            }
            else if (((Millis() - time2) >= delayToStopCurve) && isSpeedEstablished){

                stop();
                isSpeedEstablished = false;
                moveAbnormal = true;

            }
        }
    }

    else if (forcedMovement && moving){

        if (time3 == 0)
        {
            time3 = Millis();
        }
        else
        {
            if ((Millis() - time3) >= delayToStop){
                if (ABS(translationPID.getError()) <= toleranceTranslation && ABS(rotationPID.getError()) <= toleranceRotation)
                { //Stopp� pour cause de fin de mouvement
                    stop();
                    moveAbnormal = false;

                }
            }
        }
    }



    else
    {
        time = 0;
        time2 =0;
        time3 = 0; // Test
        if(moving)
            moveAbnormal = false;
    }
}

void MotionControlSystem::updatePosition() {
    static volatile int32_t lastDistance = 0;

    float deltaDistanceMm = (currentDistance - lastDistance) * TICK_TO_MM;
    lastDistance = currentDistance;

    x += (deltaDistanceMm * cos(getAngleRadian()));
    y += (deltaDistanceMm * sin(getAngleRadian()));
}


/**
 * Ordres
 */

void MotionControlSystem::orderTranslation(int32_t mmDistance) {
    translationSetpoint += (int32_t) mmDistance / TICK_TO_MM;
    if(!moving)
    {
        translationPID.resetErrors();
        moving = true;
    }
    if ( mmDistance >= 0) {
        direction = FORWARD;
        maxAcceleration = maxAccelAv;
        maxDeceleration = maxDecelAv;
        //translationPID.setTunings(this->kptav,this->kitav , this->kdtav);
    } else {
        direction = BACKWARD;
        maxAcceleration = maxAccelAr;
        maxDeceleration = maxDecelAr;
        //translationPID.setTunings(this->kptar,this->kitar , this->kdtar);
    }
    moveAbnormal = false;
}

void MotionControlSystem::orderRotation(float angleConsigneRadian, RotationWay rotationWay) {

    static int32_t deuxPiTick = (int32_t) (2 * PI / TICK_TO_RADIAN);
    static int32_t piTick = (int32_t) (PI / TICK_TO_RADIAN);

    int32_t highLevelOffset = originalAngle / TICK_TO_RADIAN;

    int32_t angleConsigneTick = angleConsigneRadian / TICK_TO_RADIAN;
    int32_t angleCourantTick = currentAngle + highLevelOffset;

    angleConsigneTick = modulo(angleConsigneTick, deuxPiTick);
    angleCourantTick = modulo(angleCourantTick, deuxPiTick);

    int32_t rotationTick = angleConsigneTick - angleCourantTick;

    if(rotationWay == FREE)
    {
        if(rotationTick >= piTick)
        {
            rotationTick -= deuxPiTick;
        }
        else if(rotationTick < -piTick)
        {
            rotationTick += deuxPiTick;
        }
    }
    else if(rotationWay == TRIGO)
    {
        if(rotationTick < 0)
        {
            rotationTick += deuxPiTick;
        }
    }
    else if(rotationWay == ANTITRIGO)
    {
        if(rotationTick > 0)
        {
            rotationTick -= deuxPiTick;
        }
    }

    rotationSetpoint = currentAngle + rotationTick;

    if(!moving)
    {
        rotationPID.resetErrors();
        moving = true;
    }
    direction = NONE;
    moveAbnormal = false;
}

void MotionControlSystem::orderCurveTrajectory(float arcLength, float curveRadius)
{

    float finalAngle = (arcLength / ABS(curveRadius)) + getAngleRadian();
    float signe = 1.0;

    if(curveRadius < 0 ) // Si le rayon de courbure est n�gatif, on tourne dans l'autre sens
        signe = -1.0;

    leftCurveRatio = (ABS(curveRadius)-(RAYON_COD_GAUCHE*signe))/(ABS(curveRadius)+RAYON_COD_DROITE-RAYON_COD_GAUCHE);
    rightCurveRatio = (ABS(curveRadius)+(RAYON_COD_DROITE*signe))/(ABS(curveRadius)+RAYON_COD_DROITE-RAYON_COD_GAUCHE);

    if(MAX(leftCurveRatio, rightCurveRatio) > 1.0)
    {
        float offset = 1.0 - MAX(leftCurveRatio, rightCurveRatio);
        leftCurveRatio = MAX(leftCurveRatio+offset,0);
        rightCurveRatio = MAX(rightCurveRatio+offset,0);
    }

    if(leftCurveRatio<0)
        leftCurveRatio=0;
    if(rightCurveRatio<0)
        rightCurveRatio=0;

    enableRotationControl(false);
    curveMovement = true;
    orderTranslation(static_cast<int32_t>(arcLength));
    orderRotation(finalAngle, FREE);
    //rotationPID.resetErrors();
}


void MotionControlSystem::orderRawPwm(Side side, int16_t pwm) {
    if (side == Side::LEFT)
        leftMotor.run(pwm);
    else
        rightMotor.run(pwm);
}

void MotionControlSystem::stop() {

    translationSetpoint = currentDistance;
    rotationSetpoint = currentAngle;
    leftSpeedSetpoint = 0;
    rightSpeedSetpoint = 0;

    if(curveMovement) //Si l'on �tait en trajectoire courbe, on remet l'asserv en rotation
        enableRotationControl(true);

    leftMotor.run(0);
    rightMotor.run(0);
    moving = false;
    curveMovement = false;
    leftCurveRatio = 1.0;
    rightCurveRatio = 1.0;
    translationPID.resetErrors();
    rotationPID.resetErrors();
    leftSpeedPID.resetErrors();
    rightSpeedPID.resetErrors();

    direction = NONE;
}


void MotionControlSystem::track()
{
    this->trackArray[trackerCursor].x = x;
    this->trackArray[trackerCursor].y = y;
    this->trackArray[trackerCursor].angle = getAngleRadian();

    this->trackArray[trackerCursor].consigneVitesseGauche = leftSpeedSetpoint;
    this->trackArray[trackerCursor].vitesseGaucheCourante = currentLeftSpeed;
    this->trackArray[trackerCursor].vitesseMoyenneGauche = averageLeftSpeed.value();
    this->trackArray[trackerCursor].pwmGauche = leftPWM;

    this->trackArray[trackerCursor].consigneVitesseDroite = rightSpeedSetpoint;
    this->trackArray[trackerCursor].vitesseDroiteCourante = currentRightSpeed;
    this->trackArray[trackerCursor].vitesseMoyenneDroite = averageRightSpeed.value();
    this->trackArray[trackerCursor].pwmDroit = rightPWM;

    this->trackArray[trackerCursor].consigneTranslation = translationSetpoint;
    this->trackArray[trackerCursor].translationCourante = currentDistance;
    this->trackArray[trackerCursor].consigneVitesseTranslation = translationSpeed;

    this->trackArray[trackerCursor].consigneRotation = rotationSetpoint;
    this->trackArray[trackerCursor].rotationCourante = currentAngle;
    this->trackArray[trackerCursor].consigneVitesseRotation = rotationSpeed;

    trackerCursor = (trackerCursor+1)%(TRACKER_SIZE);
}

void MotionControlSystem::printTrackingAll()
{
    for(int i=0; i<TRACKER_SIZE; i++)
    {
        serial.printf("%f\t%f\t%f\t",
                      trackArray[i].x, trackArray[i].y, trackArray[i].angle);
        serial.printf("%d\t%d\t%d\t%d\t",
                      trackArray[i].consigneVitesseGauche, trackArray[i].vitesseGaucheCourante, trackArray[i].vitesseMoyenneGauche, trackArray[i].pwmGauche);
        serial.printf("%d\t%d\t%d\t%d\t",
                      trackArray[i].consigneVitesseDroite, trackArray[i].vitesseDroiteCourante, trackArray[i].vitesseMoyenneDroite, trackArray[i].pwmDroit);
        serial.printf("%d\t%d\t%d\t",
                      trackArray[i].consigneTranslation , trackArray[i].translationCourante , trackArray[i].consigneVitesseTranslation);
        serial.printf("%d\t%d\t%d\t",
                      trackArray[i].consigneRotation, trackArray[i].rotationCourante, trackArray[i].consigneVitesseRotation);
        serial.printf("\r\n");
    }
}

void MotionControlSystem::printTracking() // Envoie les donn�es n�cessaires � l'analyse d'asserv / l'asserv auto (Python)
{
    for(int i=0; i<TRACKER_SIZE; i++)
    {
        serial.printf("%d\t%d\t%d\t%d\t",
                      trackArray[i].vitesseGaucheCourante, trackArray[i].vitesseDroiteCourante, trackArray[i].vitesseMoyenneGauche, trackArray[i].vitesseMoyenneDroite);
        serial.printf("%d\t%d\t%d\t%d\t",
                      trackArray[i].consigneVitesseGauche, trackArray[i].consigneVitesseDroite, trackArray[i].pwmGauche, trackArray[i].pwmDroit);
        serial.printf("\r\n");

    }
}

void MotionControlSystem::printPosition()
{
    for(int i=0; i<TRACKER_SIZE; i++)
    {
        serial.printf("%d\t%d\t%d\t",
                      trackArray[i].consigneTranslation , trackArray[i].translationCourante , trackArray[i].consigneVitesseTranslation);
        serial.printf("%d\t%d\t%d\t",
                      trackArray[i].consigneRotation, trackArray[i].rotationCourante, trackArray[i].consigneVitesseRotation);
        serial.printf("\r\n");
    }
}

void MotionControlSystem::resetTracking()
{
    trackerType zero;
    zero.angle = 0;
    zero.consigneRotation = 0;
    zero.consigneTranslation = 0;
    zero.consigneVitesseDroite = 0;
    zero.consigneVitesseGauche = 0;
    zero.consigneVitesseRotation = 0;
    zero.consigneVitesseTranslation = 0;
    zero.pwmDroit = 0;
    zero.pwmGauche = 0;
    zero.rotationCourante = 0;
    zero.translationCourante = 0;
    zero.vitesseDroiteCourante = 0;
    zero.vitesseGaucheCourante = 0;
    zero.vitesseMoyenneDroite = 0;
    zero.vitesseMoyenneGauche = 0;
    zero.x = 0;
    zero.y = 0;

    for(int i=0; i<TRACKER_SIZE; i++)
    {
        trackArray[i] = zero;
    }
    trackerCursor = 0;
}



void MotionControlSystem::testSpeed()
{
    translationControlled = false;
    rotationControlled = false;
    leftSpeedControlled = true;
    rightSpeedControlled = true;

    resetTracking();
    double testTime = (1000 * distanceTest / TICK_TO_MM / maxSpeedTranslation);
    translationSpeed = maxSpeedTranslation;
    Delay((uint32_t) testTime);
    translationSpeed = 0;
    printTracking();
    serial.printf("endtest");
    /*stop();
    Delay(1000);
    setTranslationSpeed(maxSpeedTranslation);
    translationControlled = true;
    rotationControlled = true;
    orderTranslation(-distanceTest);
    setTranslationSpeed(maxSpeedTranslation);*/

}

void MotionControlSystem::testPosition()
{
    translationControlled = true;
    rotationControlled = true;
    leftSpeedControlled = true;
    rightSpeedControlled = true;

    resetTracking();
    orderTranslation(distanceTest);
    while(moving)
    {;}
    Delay(2000);
    printPosition();
    serial.printf("endtest");

}

void MotionControlSystem::testRotation()
{
    translationControlled = true;
    rotationControlled = true;
    leftSpeedControlled = true;
    rightSpeedControlled = true;

    resetTracking();
    orderRotation(3.14, TRIGO);
    while (moving)
    {;}
    Delay(2000);
    printPosition();
    serial.printf("endtest");

}

void MotionControlSystem::longTestSpeed()
{
    translationControlled = false;
    rotationControlled = false;
    leftSpeedControlled = true;
    rightSpeedControlled = true;

    resetTracking();
    translationSpeed = 200000;
    rotationSpeed = 0;
    Delay(200);
    translationSpeed = 0;
    printTracking();
    serial.printf("endtest");

}

/**
 * Getters/Setters des constantes d'asservissement en translation/rotation/vitesse
 */

void MotionControlSystem::getTranslationTunings(float &kp, float &ki, float &kd) const {
    kp = translationPID.getKp();
    ki = translationPID.getKi();
    kd = translationPID.getKd();
}
void MotionControlSystem::getRotationTunings(float &kp, float &ki, float &kd) const {
    kp = rotationPID.getKp();
    ki = rotationPID.getKi();
    kd = rotationPID.getKd();
}
void MotionControlSystem::getLeftSpeedTunings(float &kp, float &ki, float &kd) const {
    kp = leftSpeedPID.getKp();
    ki = leftSpeedPID.getKi();
    kd = leftSpeedPID.getKd();
}
void MotionControlSystem::getRightSpeedTunings(float &kp, float &ki, float &kd) const {
    kp = rightSpeedPID.getKp();
    ki = rightSpeedPID.getKi();
    kd = rightSpeedPID.getKd();
}
void MotionControlSystem::setTranslationTunings(float kp, float ki, float kd) {
    translationPID.setTunings(kp, ki, kd);
}
void MotionControlSystem::setRotationTunings(float kp, float ki, float kd) {
    rotationPID.setTunings(kp, ki, kd);
}
void MotionControlSystem::setLeftSpeedTunings(float kp, float ki, float kd) {
    leftSpeedPID.setTunings(kp, ki, kd);
}
void MotionControlSystem::setRightSpeedTunings(float kp, float ki, float kd) {
    rightSpeedPID.setTunings(kp, ki, kd);
}

/* Definit la vitesse de translation du robot
 */
void MotionControlSystem::setTranslationSpeed(float raw_speed)
{
    //Conversion de raw_speed de mm/s en ticks/s
    double speed = raw_speed / TICK_TO_MM;

    if (speed < 0){ // SINGEPROOF
        maxSpeedTranslation = 0;
    }
    else {
        maxSpeedTranslation = (int32_t) speed;
    }
}
/* Definit la vitesse de rotation du robot
 */
void MotionControlSystem::setRotationSpeed(float raw_speed) // En Rad/s
{
    // Conversion de raw_speed de rad/s en ticks/s
    int speed = raw_speed / TICK_TO_RADIAN;
    if (speed < 0) {
        maxSpeedRotation = 0;
    }
    else {
        maxSpeedRotation = speed;
    }
}

/*
 * Getters/Setters des variables de position haut niveau
 */
float MotionControlSystem::getAngleRadian() const {
    return (currentAngle * TICK_TO_RADIAN + originalAngle);
}

void MotionControlSystem::setOriginalAngle(float angle) {
    originalAngle = angle - (getAngleRadian() - originalAngle);
}

float MotionControlSystem::getX() const{
    return x;
}

float MotionControlSystem::getY() const{
    return y;
}

void MotionControlSystem::setX(float newX){
    this->x = newX;
}

void MotionControlSystem::setY(float newY){
    this->y = newY;
}

void MotionControlSystem::resetPosition()
{
    x = 0;
    y = 0;
    setOriginalAngle(0);
    stop();
}

void MotionControlSystem::setDelayToStop(uint32_t delayToStop)
{
    this->delayToStop = delayToStop;
}

bool MotionControlSystem::isMoving() const{
    return moving;
}

bool MotionControlSystem::isMoveAbnormal() const{
    return moveAbnormal;
}

MOVING_DIRECTION MotionControlSystem::getMovingDirection() const{
    return direction;
}

Average<int32_t, 25> MotionControlSystem::getLeftSpeed()
{
    return this->averageLeftSpeed;
}
Average<int32_t, 25> MotionControlSystem::getRightSpeed()
{
    return this->averageRightSpeed;
}

float MotionControlSystem::getRightSetPoint()
{
    return this->rightSpeedSetpoint;

}

float MotionControlSystem::getLeftSetPoint()
{
    return this->leftSpeedSetpoint;
}

float MotionControlSystem::getTranslationSetPoint()
{
    return this->translationSetpoint;
}

void MotionControlSystem::getData()
{
    serial.printflnDebug("PWM:Gauche : %d __ Droit : %d ", this->leftPWM, this->rightPWM);
    serial.printflnDebug("Erreur:    Translation : %d __ PIDGauche : %d __ PIDDroit : %d" ,this->translationPID.getError(), this->leftSpeedPID.getError(), this->rightSpeedPID.getError());
    serial.printflnDebug("Dérivée:   Translation : %d __ PIDGauche : %d __ PIDDroit : %d" ,this->translationPID.getDerivativeError(), this->leftSpeedPID.getDerivativeError(), this->rightSpeedPID.getDerivativeError());
    serial.printflnDebug("Input:     Translation : %d __ PIDGauche : %d __ PIDDroit : %d" ,this->translationPID.getInput(), this->leftSpeedPID.getInput(), this->rightSpeedPID.getInput());
    serial.printflnDebug("Output:    Translation : %d __ PIDGauche : %d __ PIDDroit : %d" ,this->translationPID.getOutput(), this->leftSpeedPID.getOutput(), this->rightSpeedPID.getOutput());
    serial.printflnDebug("SetPoint:  Translation : %d __ PIDGauche : %d __ PIDDroit : %d" ,this->translationPID.getSet(), this->leftSpeedPID.getSet(), this->rightSpeedPID.getSet());
}

/*
int16_t MotionControlSystem::getMotorPWM(int i){
    if(i==0){
        return this->leftMotor.getPWM();
    }
    else if(i==1){
        return this->rightMotor.getPWM();
    }
    else {
        return 0;
    }
}*/
/*
void MotionControlSystem::getOutputs() {
    serial.printfln("%d", this->leftSpeedPID.getOutput());
    serial.printfln("%d", this->rightSpeedPID.getOutput());
}
*/
void MotionControlSystem::setAccelAv() {
    serial.printflnDebug("entrer accel avant(là : %d )", this->maxAccelAv);
    serial.read(this->maxAccelAv);
    serial.printflnDebug("entrer decel avant(là : %d )", this->maxDecelAv);
    serial.read(this->maxDecelAv);
}

void MotionControlSystem::setAccelAr() {
    serial.printflnDebug("entrer accel arrière(là : %d )", this->maxAccelAr);
    serial.read(this->maxAccelAr);
    serial.printflnDebug("entrer decel arrière(là : %d )", this->maxDecelAr);
    serial.read(this->maxDecelAr);
}
