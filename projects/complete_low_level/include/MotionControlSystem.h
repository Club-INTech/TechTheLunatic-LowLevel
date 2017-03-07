#ifndef MOTION_CONTROL_H_
#define MOTION_CONTROL_H_

#define DEBUG	1

#include "../library/Singleton.hpp"
#include "Motor.h"
#include "../library/pid.hpp"
#include <math.h>
#include "../library/delay.h"
#include "../cmsis_lib/include/misc.h"
#include "Counter.h"
#include <../library/Uart.hpp>
#include "../library/average.hpp"
#include <../library/utils.h>

#define PI 3.14159265

// 65,5mm diametre des roues
// ~1000 ticks par tour de roue
// 17cm �cartement des roues

#define RAYON_COD_GAUCHE 140
#define RAYON_COD_DROITE 144

#define TICK_TO_MM 0.2088			// unit� : mm/ticks
//#define TICK_TO_RADIAN 0.0014569	// unit� : radians/ticks
#define TICK_TO_RADIAN (TICK_TO_MM/RAYON_COD_GAUCHE)

#define AVERAGE_SPEED_SIZE	25
#define AVERAGE_DERIVATIVE_SIZE 100

#define WHEEL_DISTANCE_TO_CENTER 145.1
#define TOLERANCY 50

#if DEBUG
#define TRACKER_SIZE 		1500
#else
#define TRACKER_SIZE 		1
#endif

enum MOVING_DIRECTION {FORWARD, BACKWARD, NONE};

extern Uart<1> serial;

class MotionControlSystem : public Singleton<MotionControlSystem> {
public:

	/* Définit le sens dans lequel on doit tourner pour répondre à une consigne de rotation */
	enum RotationWay {
		FREE,        // On doit tourner dans le sens correspondant au chemin le plus court
		TRIGO,        // On doit tourner vers la droite
		ANTITRIGO    // On doit tourner vers la gauche
	};

	volatile int32_t distanceTest;


private:
	Motor leftMotor;
	Motor rightMotor;

/*
 * 		Définition des variables d'état du système (position, vitesse, consigne, ...)
 *
 * 		Les unités sont :
 * 			Pour les distances		: ticks
 * 			Pour les vitesses		: ticks/seconde
 * 			Pour les accélérations	: ticks/seconde^2
 * 			Ces unités seront vraies pour une fréquence d'asservissement de 2kHz,
 * 			si l'on souhaite changer la fréquence d'asservissement il faut adapter le calcul de la vitesse
 * 			autrement les unités ci-dessus ne seront plus valables.
 */


	//	Asservissement en vitesse du moteur droit
	PID rightSpeedPID;
	volatile int32_t rightSpeedSetpoint;    // ticks/seconde
	volatile int32_t currentRightSpeed;        // ticks/seconde
	volatile int32_t rightPWM;

	//	Asservissement en vitesse du moteur gauche
	PID leftSpeedPID;
	volatile int32_t leftSpeedSetpoint;        // ticks/seconde
	volatile int32_t currentLeftSpeed;        // ticks/seconde
	volatile int32_t leftPWM;

	//	Asservissement en position : translation
	PID translationPID;
	volatile int32_t translationSetpoint;    // ticks
	volatile int32_t currentDistance;        // ticks
	volatile int32_t translationSpeed;        // ticks/seconde

	//	Asservissement en position : rotation
	PID rotationPID;
	volatile int32_t rotationSetpoint;        // angle absolu vis� (en ticks)
	volatile int32_t currentAngle;            // ticks
	volatile int32_t rotationSpeed;            // ticks/seconde

	//	Limitation de vitesses
	volatile int32_t maxSpeed;                // definit la vitesse maximal des moteurs du robot
	volatile int32_t maxSpeedTranslation;    // definit la consigne max de vitesse de translation envoi�e au PID (trap�ze)
	volatile int32_t maxSpeedRotation;        // definit la consigne max de vitesse de rotation envoi�e au PID (trap�ze)

	//	Limitation d'acc�l�ration
	volatile int32_t maxAcceleration;

	//Les ratios de vitesse pour commander un d�placement courbe
	volatile float leftCurveRatio;
	volatile float rightCurveRatio;



	/*
	// Limitation de Jerk
	volatile int32_t maxjerk;
	*/

	//	Pour faire de jolies courbes de r�ponse du syst�me, la vitesse moyenne c'est mieux !
	Average<int32_t, AVERAGE_SPEED_SIZE> averageLeftSpeed;
	Average<int32_t, AVERAGE_SPEED_SIZE> averageRightSpeed;

	//Moyennes de la d�riv�e des erreurs (pour detecter blocages)
	Average<int32_t, AVERAGE_DERIVATIVE_SIZE> averageLeftDerivativeError;
	Average<int32_t, AVERAGE_DERIVATIVE_SIZE> averageRightDerivativeError;


/*
 * 	Variables de positionnement haut niveau (exprimm�es en unit�s pratiques ^^)
 *
 *	Pingu <3
 *
 * 	Toutes ces variables sont initialis�es � 0. Elles doivent donc �tre r�gl�es ensuite
 * 	par le haut niveau pour correspondre � son syst�me de coordonn�es.
 * 	Le bas niveau met � jour la valeur de ces variables mais ne les utilise jamais pour
 * 	lui m�me, il se contente de les transmettre au haut niveau.
 */
	volatile float x;                // Positionnement 'x' (mm)
	volatile float y;                // Positionnement 'y' (mm)
	volatile float originalAngle;    // Angle d'origine	  (radians)
	// 'originalAngle' repr�sente un offset ajout� � l'angle courant pour que nos angles en radians co�ncident avec la repr�sentation haut niveau des angles.


	// Variables d'�tat du mouvement
	volatile bool moving;
	volatile MOVING_DIRECTION direction;
	volatile bool moveAbnormal;

	// Variables d'activation des diff�rents PID
	volatile bool translationControlled;
	volatile bool rotationControlled;
	volatile bool leftSpeedControlled;
	volatile bool rightSpeedControlled;


	volatile bool curveMovement;
	volatile bool forcedMovement; // Si true, alors pas de gestion de l'arret : ON FORCE MODAFUCKA !!!

	// Variables de r�glage de la d�tection de blocage physique
	unsigned int delayToStop;  //En ms
	unsigned int delayToStopCurve;

	//Nombre de ticks de tol�rance pour consid�rer qu'on est arriv� � destination
	int toleranceTranslation;
	int toleranceRotation;

	int toleranceSpeed; // Tol�rance avant de consid�rer le mouvement anormal (�cart entre la consigne de vitesse et la vitesse r�elle)
	int toleranceSpeedEstablished; // Tol�rance autour de la vitesse �tablie avant de capter un blocage

	int toleranceDifferentielle;

	int delayToEstablish; // Temps � attendre avant de consid�rer la vitesse stable

	float toleranceCurveRatio; // Tol�rance en trajectoire courbe avant de bloquer si le rayon est mauvais
	/*
	 * Dispositif d'enregistrement de l'�tat du syst�me pour permettre le d�bug
	 * La valeur de TRACKER_SIZE d�pend de la valeur de DEBUG.
	 */
	struct trackerType {
		float x;
		float y;
		float angle;

		int consigneVitesseGauche;
		int vitesseGaucheCourante;
		int vitesseMoyenneGauche;
		int pwmGauche;

		int consigneVitesseDroite;
		int vitesseDroiteCourante;
		int vitesseMoyenneDroite;
		int pwmDroit;

		int consigneTranslation;
		int translationCourante;
		int consigneVitesseTranslation;

		int consigneRotation;
		int rotationCourante;
		int consigneVitesseRotation;
	};

	trackerType trackArray[TRACKER_SIZE];
	unsigned int trackerCursor;

	bool isPhysicallyStopped();//Indique si le robot est immobile.
	bool isLeftWheelSpeedAbnormal();

	bool isRightWheelSpeedAbnormal();

public:
	MotionControlSystem();

	void init();

	void setRawPositiveTranslationSpeed();

	void setRawNegativeTranslationSpeed();

	void setRawPositiveRotationSpeed();

	void setRawNegativeRotationSpeed();

	void setRawNullSpeed();

	void control();

	void updatePosition();

	void manageStop();

	void enableForcedMovement();

	void disableForcedMovement();


	void track();//Stock les valeurs de d�bug
	void printTrackingAll();//Affiche l'int�gralit� du tableau de tracking
	void printTracking(); // Envoie des donn�es pour l'asserv auto
	void printPosition();

	void resetTracking();// Reset le tableau de tracking


	void enable(bool);

	void enableTranslationControl(bool);

	void enableRotationControl(bool);

	void enableSpeedControl(bool);

	void orderTranslation(int32_t);

	void orderRotation(float, RotationWay);

	void orderRawPwm(Side, int16_t);

	void orderCurveTrajectory(float, float);

	void stop();

	void setTranslationTunings(float, float, float);

	void setRotationTunings(float, float, float);

	void setLeftSpeedTunings(float, float, float);

	void setRightSpeedTunings(float, float, float);

	void getTranslationTunings(float &, float &, float &) const;

	void getRotationTunings(float &, float &, float &) const;

	void getLeftSpeedTunings(float &, float &, float &) const;

	void getRightSpeedTunings(float &, float &, float &) const;

	float getAngleRadian() const;

	void setOriginalAngle(float);

	float getX() const;

	float getY() const;

	void setX(float);

	void setY(float);

	void resetPosition(void);

	void setDelayToStop(uint32_t);

	void setTranslationSpeed(float);

	void setRotationSpeed(float);

	bool isMoving() const;

	bool isMoveAbnormal() const;

	MOVING_DIRECTION getMovingDirection() const;

	void setTestSpeed(int32_t);

	void testSpeed();

	void testSpeedReverse();

	void longTestSpeed();

	void testPosition();

	void testRotation();

	Average<int32_t, 25> getLeftSpeed();

	Average<int32_t, 25> getRightSpeed();

	float getRightSetPoint();

	float getLeftSetPoint();

	float getTranslationSetPoint();

    void getData();
};

#endif /* MOTION_CONTROL_H_ */
