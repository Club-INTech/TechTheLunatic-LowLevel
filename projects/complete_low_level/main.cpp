#include "Uart.hpp"
#include "MotionControlSystem.h"
#include "delay.h"
#include "ActuatorsMgr.hpp"
#include "SensorMgr.h"
#include "voltage_controller.hpp"
#include "BinaryMotorMgr.hpp"


int main(void)
{
	Delay_Init();
	Uart<1> serial;
	Uart<2> serial_ax;
	serial.init(115200);
	serial_ax.init(9600);
	serial_ax.disable_rx();

	MotionControlSystem* motionControlSystem = &MotionControlSystem::Instance(); // motionControlSystem est tout simplement un pointeur vers une référence d'un objet de type MotionControlSystem #TRIVIAL #USELESS
	motionControlSystem->init();
	ActuatorsMgr* actuatorsMgr = &ActuatorsMgr::Instance();
	SensorMgr* sensorMgr = &SensorMgr::Instance();
	BinaryMotorMgr* binaryMotorMgr = &BinaryMotorMgr::Instance();
	Voltage_controller* voltage = &Voltage_controller::Instance();

	char order[64];//Permet le stockage du message reçu par la liaison série

	bool translation = true;//permet de basculer entre les réglages de cte d'asserv en translation et en rotation

	while(1)
	{
		sensorMgr->refresh(motionControlSystem->getMovingDirection());

		uint8_t tailleBuffer = serial.available();

		if (tailleBuffer && tailleBuffer < RX_BUFFER_SIZE - 1)
		{
			serial.read(order);
			serial.printfln("_");//Acquittement

			if(!strcmp("?",order))				//Ping
			{
				serial.printfln("0");
			}
			else if(!strcmp("f",order))			//Indiquer l'état du mouvement du robot
			{
				serial.printfln("%d", motionControlSystem->isMoving());//Robot en mouvement ou pas ?
				serial.printfln("%d", motionControlSystem->isMoveAbnormal());//Cet état du mouvement est il anormal ?
			}
			else if(!strcmp("?xyo",order))		//Indiquer la position du robot (en mm et radians)
			{
				serial.printfln("%f", motionControlSystem->getX());
				serial.printfln("%f", motionControlSystem->getY());
				serial.printfln("%f", motionControlSystem->getAngleRadian());
			}
			else if(!strcmp("d", order))		//Ordre de déplacement rectiligne (en mm)
			{
				int deplacement = 0;
				serial.read(deplacement);
				serial.printfln("_");//Acquittement
				motionControlSystem->orderTranslation(deplacement);
			}
			else if(!strcmp("t", order))		//Ordre de rotation via un angle absolu (en radians)
			{
				float angle = motionControlSystem->getAngleRadian();
				serial.read(angle);
				serial.printfln("_");//Acquittement
				motionControlSystem->orderRotation(angle, MotionControlSystem::FREE);
			}

			else if(!strcmp("tor", order))  // Ordre de rotation seulement à DROITE (pour ne pas perdre le sable)
			{
				float angle = motionControlSystem->getAngleRadian();
				serial.read(angle);
				serial.printfln("_");//Acquittement
				motionControlSystem->orderRotation(angle, MotionControlSystem::ANTITRIGO);
			}

			else if(!strcmp("tol", order))  // Ordre de rotation seulement à GAUCHE (pour ne pas perdre le sable)
			{
				float angle = motionControlSystem->getAngleRadian();
				serial.read(angle);
				serial.printfln("_");//Acquittement
				motionControlSystem->orderRotation(angle, MotionControlSystem::TRIGO);
			}

			else if(!strcmp("dc", order)) //Rotation + translation = trajectoire courbe !
			{
				float arcLenght = 0;
				float curveRadius = 0;
				serial.read(arcLenght);
				serial.printfln("_");//Acquittement
				serial.read(curveRadius);
				serial.printfln("_");//Acquittement
				motionControlSystem->orderCurveTrajectory(arcLenght, curveRadius);
			}

			else if(!strcmp("efm", order)) // Activer les mouvements forcés (sans blocage)
			{
				motionControlSystem->enableForcedMovement();
			}

			else if(!strcmp("dfm", order)) // désactive le forçage
			{
				motionControlSystem->disableForcedMovement();
			}


			else if(!strcmp("t3", order))		//Ordre de rotation via un angle relatif (en radians)
			{
				float angle_actuel = motionControlSystem->getAngleRadian(), delta_angle = 0;
				serial.read(delta_angle);
				serial.printfln("_");
				motionControlSystem->orderRotation(angle_actuel + delta_angle, MotionControlSystem::FREE);
			}
			else if(!strcmp("r", order))		//Ordre de rotation via un angle relatif (en degrés)
			{
				float angle_actuel = motionControlSystem->getAngleRadian()*180/PI, delta_angle = 0;
				serial.read(delta_angle);
				serial.printfln("_");
				motionControlSystem->orderRotation((angle_actuel + delta_angle)*PI/180, MotionControlSystem::FREE);
			}
			else if(!strcmp("stop",order))		//Ordre d'arrêt (asservissement à la position actuelle)
			{
				motionControlSystem->stop();
			}
			else if(!strcmp("us",order))		//Indiquer la distance mesurée par les capteurs à ultrason
			{
				serial.printfln("%d", sensorMgr->getSensorDistanceAVG());//en mm
				serial.printfln("%d", sensorMgr->getSensorDistanceAVD());//en mm
				serial.printfln("%d", sensorMgr->getSensorDistanceARG());//en mm // qui représente AVD en fait
				serial.printfln("%d", sensorMgr->getSensorDistanceARD());//en mm // qui représente l'arrière.
				//serial.printfln("0"); // Pour que le haut niveau gueule pas en disant "OMG IL Y A QUE 3 REPONSES SUR 4 !" #Mongol
			}
			else if(!strcmp("j",order))			//Indiquer l'état du jumper (0='en place'; 1='dehors')
			{
				serial.printfln("%d", sensorMgr->isJumperOut());
			}
			else if(!strcmp("ct0",order))		//Désactiver l'asservissement en translation
			{
				motionControlSystem->enableTranslationControl(false);
			}
			else if(!strcmp("ct1",order))		//Activer l'asservissement en translation
			{
				motionControlSystem->enableTranslationControl(true);
			}
			else if(!strcmp("cr0",order))		//Désactiver l'asservissement en rotation
			{
				motionControlSystem->enableRotationControl(false);
			}
			else if(!strcmp("cr1",order))		//Activer l'asservissement en rotation
			{
				motionControlSystem->enableRotationControl(true);
			}
			else if(!strcmp("cv0",order))		//Activer l'asservissement en rotation
			{
				motionControlSystem->enableSpeedControl(false);
			}
			else if(!strcmp("cv1",order))		//Activer l'asservissement en rotation
			{
				motionControlSystem->enableSpeedControl(true);
			}

			else if(!strcmp("cx",order))		//Régler la composante x de la position (en mm)
			{
				float x;
				serial.read(x);
				serial.printfln("_");//Acquittement
				motionControlSystem->setX(x);
			}
			else if(!strcmp("cy",order))		//Régler la composante y de la position (en mm)
			{
				float y;
				serial.read(y);
				serial.printfln("_");//Acquittement
				motionControlSystem->setY(y);
			}
			else if(!strcmp("co",order))		//Régler l'orientation du robot (en radians)
			{
				float o;
				serial.read(o);
				serial.printfln("_");//Acquittement
				motionControlSystem->setOriginalAngle(o);
			}
			else if(!strcmp("ctv",order))   //Régler la vitesse de translation
			{
				float speed = 0; // unité de speed : mm/s
				serial.read(speed);
				serial.printfln("_");
				motionControlSystem->setTranslationSpeed(speed);
			}
			else if(!strcmp("crv",order))  //Régler la vitesse de rotation
			{
				float speedRotation = 0; // rad/s
				serial.read(speedRotation);
				serial.printfln("_");
				motionControlSystem->setRotationSpeed(speedRotation);
			}

			else if(!strcmp("ssa", order))
			{
				motionControlSystem->setSmoothAcceleration();
			}

			else if(!strcmp("sva", order))
			{
				motionControlSystem->setViolentAcceleration();
			}

			// POUR MONTLHERY

			else if(!strcmp("montlhery", order))
			{
				motionControlSystem->enableTranslationControl(false);
				motionControlSystem->enableRotationControl(false);
			}

			else if(!strcmp("av", order))
			{
				motionControlSystem->setRawPositiveTranslationSpeed();
			}

			else if(!strcmp("rc", order))
			{
				motionControlSystem->setRawNegativeTranslationSpeed();
			}

			else if(!strcmp("td", order))
			{
				motionControlSystem->setRawNegativeRotationSpeed();
			}

			else if(!strcmp("tg", order))
			{
				motionControlSystem->setRawPositiveRotationSpeed();
			}

			else if(!strcmp("sstop", order)) // Stoppe l'asserv en vitesse
			{
				motionControlSystem->setRawNullSpeed();
			}


/*			 __________________
 * 		   *|                  |*
 *		   *|COMMANDES DE DEBUG|*
 *		   *|__________________|*
 */
			else if(!strcmp("!",order))//Test quelconque
			{

			}
			else if(!strcmp("oxy",order))
			{
				serial.printfln("x=%f\r\ny=%f", motionControlSystem->getX(), motionControlSystem->getY());
				serial.printfln("o=%f", motionControlSystem->getAngleRadian());
			}
			else if (!strcmp("at", order))	// Commute l'asservissement en translation
			{
				static bool asservTranslation = false;
				motionControlSystem->enableTranslationControl(asservTranslation);
				serial.printfln("l'asserv en translation est désormais");
				if (asservTranslation)
				{
					serial.printfln("asservi en translation");
				}
				else
				{
					serial.printfln("non asservi en translation");
				}
				asservTranslation = !asservTranslation;
			}
			else if (!strcmp("ar", order)) // Commute l'asservissement en rotation
			{
				static bool asservRotation = false;
				motionControlSystem->enableRotationControl(asservRotation);
				serial.printfln("l'asserv en rotation est désormais");
				if (asservRotation)
				{
					serial.printfln("asservi en rotation");
				}
				else
				{
					serial.printfln("non asservi en rotation");
				}
				asservRotation = !asservRotation;
			}

			else if(!strcmp("rp",order))//Reset position
			{
				motionControlSystem->resetPosition();
				serial.printfln("Reset position");
			}


			else if(!strcmp("testSpeed",order))
			{
				motionControlSystem->testSpeed();
			}
			else if(!strcmp("dtest",order))
			{
				int distance = 0;
				serial.printfln("Distance du test : (mm)");
				serial.read(distance);
				motionControlSystem->distanceTest = distance;
			}

			else if(!strcmp("continualTest",order))//Test long
			{
				motionControlSystem->longTestSpeed();
			}
			else if(!strcmp("testPosition",order))
			{
				motionControlSystem->testPosition();
			}

			else if(!strcmp("testRotation",order))
			{
				motionControlSystem->testRotation();
			}

			else if(!strcmp("av", order)) // desactive asserv vitesse
			{
				static bool asservVitesse = false;
				motionControlSystem->enableSpeedControl(asservVitesse);
				serial.printf("L'asserv en vitesse est ");
				if(!asservVitesse)
				{
					serial.printfln("desactivée");
				}
				else {
					serial.printfln("activée");
				}
			}



/*			 ___________________
 * 		   *|                   |*
 *		   *|CONSTANTES D'ASSERV|*
 *		   *|___________________|*
 */


			else if(!strcmp("toggle",order))//Bascule entre le réglage d'asserv en translation et en rotation
			{
				translation = !translation;
				if(translation)
					serial.printfln("reglage de la transation");
				else
					serial.printfln("reglage de la rotation");
			}
			else if(!strcmp("display",order))
			{
				float
					kp_t, ki_t, kd_t,	// Translation
					kp_r, ki_r, kd_r,	// Rotation
					kp_g, ki_g, kd_g,	// Vitesse gauche
					kp_d, ki_d, kd_d;	// Vitesse droite
				motionControlSystem->getTranslationTunings(kp_t, ki_t, kd_t);
				motionControlSystem->getRotationTunings(kp_r, ki_r, kd_r);
				motionControlSystem->getLeftSpeedTunings(kp_g, ki_g, kd_g);
				motionControlSystem->getRightSpeedTunings(kp_d, ki_d, kd_d);
				serial.printfln("trans : kp= %g ; ki= %g ; kd= %g", kp_t, ki_t, kd_t);
				serial.printfln("rot   : kp= %g ; ki= %g ; kd= %g", kp_r, ki_r, kd_r);
				serial.printfln("gauche: kp= %g ; ki= %g ; kd= %g", kp_g, ki_g, kd_g);
				serial.printfln("droite: kp= %g ; ki= %g ; kd= %g", kp_d, ki_d, kd_d);
			}

			else if(!strcmp("autoasserv" ,order))// Commande pour le programme d'autoasserv (python)
			{
				float
					kp_g, kp_d, ki_g, ki_d, kd_g, kd_d;

				motionControlSystem->getLeftSpeedTunings(kp_g, ki_g, kd_g);
				motionControlSystem->getRightSpeedTunings(kp_d, ki_d, kd_d);

				motionControlSystem->printTracking();
				serial.printf("endtest");
			}


			else if(!strcmp("dts",order))//Delay To Stop
			{
				uint32_t delayToStop = 0;
				serial.printfln("Delay to stop ? (ms)");
				serial.read(delayToStop);
				motionControlSystem->setDelayToStop(delayToStop);
				serial.printfln("Delay to stop = %d", delayToStop);
			}

// ***********  TRANSLATION  ***********
			else if(!strcmp("kpt",order))
			{
				float kp, ki, kd;
				serial.printfln("kp_trans ?");
				motionControlSystem->getTranslationTunings(kp,ki,kd);
				serial.read(kp);
				motionControlSystem->setTranslationTunings(kp,ki,kd);
				serial.printfln("kp_trans = %g", kp);
			}
			else if(!strcmp("kdt",order))
			{
				float kp, ki, kd;
				serial.printfln("kd_trans ?");
				motionControlSystem->getTranslationTunings(kp,ki,kd);
				serial.read(kd);
				motionControlSystem->setTranslationTunings(kp,ki,kd);
				serial.printfln("kd_trans = %g", kd);
			}
			else if(!strcmp("kit",order))
			{
				float kp, ki, kd;
				serial.printfln("ki_trans ?");
				motionControlSystem->getTranslationTunings(kp,ki,kd);
				serial.read(ki);
				motionControlSystem->setTranslationTunings(kp,ki,kd);
				serial.printfln("ki_trans = %g", ki);
			}

// ***********  ROTATION  ***********
			else if(!strcmp("kpr",order))
			{
				float kp, ki, kd;
				serial.printfln("kp_rot ?");
				motionControlSystem->getRotationTunings(kp,ki,kd);
				serial.read(kp);
				motionControlSystem->setRotationTunings(kp,ki,kd);
				serial.printfln("kp_rot = %g", kp);
			}
			else if(!strcmp("kir",order))
			{
				float kp, ki, kd;
				serial.printfln("ki_rot ?");
				motionControlSystem->getRotationTunings(kp,ki,kd);
				serial.read(ki);
				motionControlSystem->setRotationTunings(kp,ki,kd);
				serial.printfln("ki_rot = %g", ki);
			}
			else if(!strcmp("kdr",order))
			{
				float kp, ki, kd;
				serial.printfln("kd_rot ?");
				motionControlSystem->getRotationTunings(kp,ki,kd);
				serial.read(kd);
				motionControlSystem->setRotationTunings(kp,ki,kd);
				serial.printfln("kd_rot = %g", kd);
			}

// ***********  VITESSE GAUCHE  ***********
			else if(!strcmp("kpg",order))
			{
				float kp, ki, kd;
				serial.printfln("kp_gauche ?");
				motionControlSystem->getLeftSpeedTunings(kp,ki,kd);
				serial.read(kp);
				motionControlSystem->setLeftSpeedTunings(kp,ki,kd);
				serial.printfln("kp_gauche = %g", kp);
			}
			else if(!strcmp("kig",order))
			{
				float kp, ki, kd;
				serial.printfln("ki_gauche ?");
				motionControlSystem->getLeftSpeedTunings(kp,ki,kd);
				serial.read(ki);
				motionControlSystem->setLeftSpeedTunings(kp,ki,kd);
				serial.printfln("ki_gauche = %g", ki);
			}
			else if(!strcmp("kdg",order))
			{
				float kp, ki, kd;
				serial.printfln("kd_gauche ?");
				motionControlSystem->getLeftSpeedTunings(kp,ki,kd);
				serial.read(kd);
				motionControlSystem->setLeftSpeedTunings(kp,ki,kd);
				serial.printfln("kd_gauche = %g", kd);
			}

// ***********  VITESSE DROITE  ***********
			else if(!strcmp("kpd",order))
			{
				float kp, ki, kd;
				serial.printfln("kp_droite ?");
				motionControlSystem->getRightSpeedTunings(kp,ki,kd);
				serial.read(kp);
				motionControlSystem->setRightSpeedTunings(kp,ki,kd);
				serial.printfln("kp_droite = %g", kp);
			}
			else if(!strcmp("kid",order))
			{
				float kp, ki, kd;
				serial.printfln("ki_droite ?");
				motionControlSystem->getRightSpeedTunings(kp,ki,kd);
				serial.read(ki);
				motionControlSystem->setRightSpeedTunings(kp,ki,kd);
				serial.printfln("ki_droite = %g", ki);
			}
			else if(!strcmp("kdd",order))
			{
				float kp, ki, kd;
				serial.printfln("kd_droite ?");
				motionControlSystem->getRightSpeedTunings(kp,ki,kd);
				serial.read(kd);
				motionControlSystem->setRightSpeedTunings(kp,ki,kd);
				serial.printfln("kd_droite = %g", kd);
			}



	/**
	 * 		Commandes de tracking des variables du système (débug)
	 */
			else if(!strcmp("trackAll",order))
			{
				motionControlSystem->printTrackingAll();
			}

			else if(!strcmp("adc", order)) // Pour tester la tension d'alimentation selon l'adc
			{
				serial.printfln("%d", voltage->test());
			}



/*			 __________________
 * 		   *|                  |*
 *		   *|	ACTIONNEURS	   |*
 *		   *|__________________|*
 */

			/* --- AX12 ---*/

			else if(!strcmp("setallid",order))
			{
				actuatorsMgr->setAllID();
			}

			else if(!strcmp("fprh",order)) // Descente du bras droit aimanté (poissons)
			{
				actuatorsMgr->fishingRightUp();
			}

			else if(!strcmp("fprm",order)) // Descente du bras droit aimanté (poissons)
			{
				actuatorsMgr->fishingRightMid();
			}

			else if(!strcmp("fprl",order)) // Descente du bras droit aimanté (poissons)
			{
				actuatorsMgr->fishingRightDown();
			}

			else if(!strcmp("mpr",order))
			{
				actuatorsMgr->midPositionRight();
			}


			else if(!strcmp("rmd",order)) // rightMagnetsDown
			{
				actuatorsMgr->rightMagnetsDown();

			}
			else if(!strcmp("rfd",order))	// rightFingerDown
			{
				actuatorsMgr->rightFingerDown();

			}
			else if(!strcmp("rmu",order))	//rightMagnetsUp
			{
				actuatorsMgr->rightMagnetsUp();

			}
			else if(!strcmp("rfu",order))	//rightFingerUp
			{
				actuatorsMgr->rightFingerUp();

			}

			else if(!strcmp("fplm",order)) // Descente du bras gauche aimanté (poissons)
			{
				actuatorsMgr->fishingLeftMid();
			}
			else if(!strcmp("fplh",order)) // Descente du bras gauche aimanté (poissons)
			{
				actuatorsMgr->fishingLeftUp();
			}
			else if(!strcmp("fpll",order)) // Descente du bras gauche aimanté (poissons)
			{
				actuatorsMgr->fishingLeftDown();
			}

			else if(!strcmp("mpl",order))
			{
				actuatorsMgr->midPositionLeft();
			}

			else if(!strcmp("lmd",order)) // leftMagnetsDown
			{
				actuatorsMgr->leftMagnetsDown();

			}
			else if(!strcmp("lfd",order))	// leftFingerDown
			{
				actuatorsMgr->leftFingerDown();

			}
			else if(!strcmp("lmu",order))	//leftMagnetsUp
			{
				actuatorsMgr->leftMagnetsUp();

			}
			else if(!strcmp("lfu",order))	//leftFingerUp
			{
				actuatorsMgr->leftFingerUp();

			}

			else if(!strcmp("emr", order)) // permet de tester manuellement les positions des AX12
			{
				int position = 150;
				serial.printfln("Entrez angle");
				serial.read(position);
				serial.printfln("angle = %d", position);
				if(position >= 0 && position <= 300)
					actuatorsMgr->setAXposMagnetsRight(position);
				serial.printfln("done.");

			}

			else if(!strcmp("efr", order)) // permet de tester manuellement les positions des AX12
			{
				int position = 150;
				serial.printfln("Entrez angle");
				serial.read(position);
				serial.printfln("angle = %d", position);
				if(position >= 0 && position <= 300)
					actuatorsMgr->setAXposFreeRightFishes(position);
				serial.printfln("done.");

			}

			else if(!strcmp("eml", order)) // permet de tester manuellement les positions des AX12
			{
				int position = 150;
				serial.printfln("Entrez angle");
				serial.read(position);
				serial.printfln("angle = %d", position);
				if(position >= 0 && position <= 300)
					actuatorsMgr->setAXposMagnetsLeft(position);
				serial.printfln("done.");

			}

			else if(!strcmp("efl", order)) // permet de tester manuellement les positions des AX12
			{
				int position = 150;
				serial.printfln("Entrez angle");
				serial.read(position);
				serial.printfln("angle = %d", position);
				if(position >= 0 && position <= 300)
					actuatorsMgr->setAXposFreeLeftFishes(position);
				serial.printfln("done.");

			}

			else if(!strcmp("aif", order)) {
				actuatorsMgr->initialPositionFish();
			}

			else if(!strcmp("caxs", order)) { //commande de debug
				int speed = 100;
				serial.printfln("Entrez vitesse");
				serial.read(speed);
				actuatorsMgr->changeAXSpeed(speed);
				serial.printfln("Done");
			}


			/* --- Portes sable ---*/

			else if(!strcmp("odl", order)) {
				bool value = sensorMgr->isLeftDoorOpen();
				if(!value){ // Si la porte n'est pas en fin de course ouverte
					binaryMotorMgr->setLeftDoorOpening(true);
					binaryMotorMgr->runForwardLeft(); //ouvrir

				}
			}

			else if(!strcmp("odr", order)) {
				bool value = sensorMgr->isRightDoorOpen();
				if(!value) {
					binaryMotorMgr->setRightDoorOpening(true);
					binaryMotorMgr->runForwardRight();
				}
			}

			else if(!strcmp("cdl", order)) {
				bool value = sensorMgr->isLeftDoorClosed();
					if(!value) {
						binaryMotorMgr->setLeftDoorClosing(true);
						binaryMotorMgr->runBackwardLeft();
					}
			}

			else if(!strcmp("cdr", order)) {
				bool value = sensorMgr->isRightDoorClosed();
					if(!value) {
						binaryMotorMgr->setRightDoorClosing(true);
						binaryMotorMgr->runBackwardRight();
					}
			}

			else if(!strcmp("sdr", order)) {
				binaryMotorMgr->stopRightDoor();
			}

			else if(!strcmp("sdl", order)) {
				binaryMotorMgr->stopLeftDoor();
			}


			/* Axes rotatifs */

			else if(!strcmp("ral", order)) {
				binaryMotorMgr->runAxisLeft();
			}

			else if(!strcmp("rar", order)) {
				binaryMotorMgr->runAxisRight();
			}

			else if(!strcmp("sal", order)) {
				binaryMotorMgr->stopAxisLeft();
			}

			else if(!strcmp("sar", order)) {
				binaryMotorMgr->stopAxisRight();
			}

			else if(!strcmp("irdo", order)) { // is Right Door Open
				bool door = sensorMgr->isRightDoorOpen();
				serial.printfln("%d", door);
			}

			else if(!strcmp("ildo", order)) {
				bool door = sensorMgr->isLeftDoorOpen();
				serial.printfln("%d", door);
			}

			else if(!strcmp("irdc", order)) {
				bool door = sensorMgr->isRightDoorClosed();
				serial.printfln("%d", door);
			}

			else if(!strcmp("ildc", order)) {
				bool door = sensorMgr->isLeftDoorClosed();
				serial.printfln("%d", door);
			}

			else if(!strcmp("irdb", order)){
				bool blocked = binaryMotorMgr->isRightDoorBlocked();
				serial.printfln("%d", blocked);
			}

			else if(!strcmp("ildb", order)){
				bool blocked = binaryMotorMgr->isLeftDoorBlocked();
				serial.printfln("%d", blocked);
			}


/*			 __________________
 * 		   *|                  |*
 *		   *|  ERREURS DE COM  |*
 *		   *|__________________|*
 */


			else if(!strcmp("uoe",order)) // test d'un mauvais retour bas niveau --> haut niveau
			{ // test
				serial.printfln("Une fraise");
			}

			// Sinon, Ordre inconnu

			else
			{
				serial.printfln("Ordre inconnu");
			}

		}
#if DEBUG
		else if(tailleBuffer == RX_BUFFER_SIZE - 1)
		{
			serial.printfln("CRITICAL OVERFLOW !");
			motionControlSystem->enableTranslationControl(false);
			motionControlSystem->enableRotationControl(false);
			motionControlSystem->enableSpeedControl(false);

			while(true)
				;
		}
#endif
	}
}

extern "C" {
//Interruption overflow TIMER4
void TIM4_IRQHandler(void) { //2kHz = 0.0005s = 0.5ms
	volatile static uint32_t i = 0, j = 0, k = 0;
	static MotionControlSystem* motionControlSystem = &MotionControlSystem::Instance();
	static BinaryMotorMgr* binaryMotorMgr = &BinaryMotorMgr::Instance();
	static Voltage_controller* voltage = &Voltage_controller::Instance();

	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		//Remise à 0 manuelle du flag d'interruption nécessaire
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		//Asservissement et mise à jour de la position
		motionControlSystem->control();
		motionControlSystem->updatePosition();


		if(j >= 5){ //2.5ms
			motionControlSystem->track();
			motionControlSystem->manageStop();
			binaryMotorMgr->manageBlockedDoor();
			j=0;
		}

		if(k >= 200)
		{
			voltage->measure();
			k=0;
		}

		k++;
		i++;
		j++;
	}
}

void EXTI9_5_IRQHandler(void)
{
	static SensorMgr* sensorMgr = &SensorMgr::Instance(); // Capteurs US
/*
	//Interruptions de l'ultrason de test
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
        sensorMgr->sensorInterrupt(5);

        // Clear interrupt flag
        EXTI_ClearITPendingBit(EXTI_Line5);

    }
*/

    if (EXTI_GetITStatus(EXTI_Line7) != RESET) {
    	sensorMgr->AVGInterrupt();
        //serial.printfln("7");


        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET) {
           sensorMgr->ARDInterrupt();
           //serial.printfln("6");

           /* Clear interrupt flag */
           EXTI_ClearITPendingBit(EXTI_Line6);
       }

    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
    	sensorMgr->AVDInterrupt();
        //serial.printfln("5");


    	/* Clear interrupt flag */
    	EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
    	while(42){
    		}
			   EXTI_ClearITPendingBit(EXTI_Line8);
		   }

    if (EXTI_GetITStatus(EXTI_Line9) != RESET) {
    	while(42){
    		}
    		EXTI_ClearITPendingBit(EXTI_Line9);
           }

}

void EXTI0_IRQHandler(void) // Capteur fin de course Droite ouverte
{

	static BinaryMotorMgr* binaryMotorMgr = &BinaryMotorMgr::Instance();

	if(!binaryMotorMgr->isRightDoorClosing()){ // Sécurité : N'arrete pas le moteur il est en train d'ouvrir (problème de front montant du capteur)
		binaryMotorMgr->stopRightDoor(); // stopper sur front montant
		binaryMotorMgr->setRightDoorOpening(false);
	}
	EXTI_ClearITPendingBit(EXTI_Line0);

}

void EXTI15_10_IRQHandler(void)
{
	static BinaryMotorMgr* binaryMotorMgr = &BinaryMotorMgr::Instance();

	if(EXTI_GetITStatus(EXTI_Line13) != RESET) { // Droite fermée
		if(!binaryMotorMgr->isRightDoorOpening()){

			binaryMotorMgr->stopRightDoor();
			binaryMotorMgr->setRightDoorClosing(false);
		}
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
	else if(EXTI_GetITStatus(EXTI_Line15) != RESET) { // Gauche ouverte
		if(!binaryMotorMgr->isLeftDoorClosing()){

			binaryMotorMgr->stopLeftDoor();
			binaryMotorMgr->setLeftDoorOpening(false);
		}
		EXTI_ClearITPendingBit(EXTI_Line15);
	}

	if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
		while(42){
			}
		EXTI_ClearITPendingBit(EXTI_Line10);
	           }
	if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
		while(42){
			}
		EXTI_ClearITPendingBit(EXTI_Line11);
	           }
	if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
		while(42){
			}
		EXTI_ClearITPendingBit(EXTI_Line12);
	           }
	if (EXTI_GetITStatus(EXTI_Line14) != RESET) {
		while(42){
			}
		EXTI_ClearITPendingBit(EXTI_Line14);
	           }


}


void EXTI1_IRQHandler(void) // Gauche fermée
{

	static BinaryMotorMgr* binaryMotorMgr = &BinaryMotorMgr::Instance();

	if(!binaryMotorMgr->isLeftDoorOpening())
	{
		binaryMotorMgr->stopLeftDoor();
		binaryMotorMgr->setLeftDoorClosing(false);
	}
	EXTI_ClearITPendingBit(EXTI_Line1);

}




void EXTI4_IRQHandler(void) // Capteur AVD (celui qui a foutu la merde, et qui est sensé être en PA5)
{
	static SensorMgr* sensorMgr = &SensorMgr::Instance();

	if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
	        sensorMgr->ARGInterrupt();
	          // serial.printfln("4");
	       // serial.printfln("interrupt 4");
	        // Clear interrupt flag
	        EXTI_ClearITPendingBit(EXTI_Line4);
	    }
}

void EXTI2_IRQHandler(void)
{

}

void EXTI3_IRQHandler(void)
{
	while(42){
		}
	EXTI_ClearITPendingBit(EXTI_Line3);
}

void HardFault_Handler(void)
{
	while(1);
}



/*
 *   Dead Pingu in the Main !
 *      	  . --- .
		    /        \
		   |  X  _  X |
		   |  ./   \. |
		   /  `-._.-'  \
		.' /         \ `.
	.-~.-~/    o   o  \~-.~-.
.-~ ~    |    o  o     |    ~ ~-.
`- .     |      o  o   |     . -'
	 ~ - |      o      | - ~
	 	 \             /
	 	__\           /___
	 	~;_  >- . . -<  _i~
	 	  `'         `'
*/
}
