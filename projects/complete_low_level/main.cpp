#include <stm32f4xx_gpio.h>
#include <ElevatorMgr.h>
#include "ActuatorsMgr.hpp"
#include "library/voltage_controller.hpp"

bool autoUpdatePosition = false; // active le mode d'envoi automatique de position au haut niveau

/**Contient la boucle principale de gestion des entrées série du programme
 *
 * @author caillou, sylvain, rémi, melanie, Ug
 **/

int main(void)
{
    Delay_Init(); //on initialise le système de minuterie de la STM32 (SysTick)
    //SysTicks compte à l'envers depuis la valeur qu'on lui donne dans SysTick_Config() jusqu'à 0
    //cela permet d'avoir des interruptions (ici toutes les us)
    Uart<1> serial; //initialise l'uart1 qui permet la communication série
    Uart<2> serial_ax; //initialise l'uart2 qui permet la communication avec les ax12
    serial.init(115200); //initialisation de la vitesse de transmission
    serial_ax.init(9600);
    serial_ax.disable_rx(); //on décide de ne pas recevoir des ax12

    //on créé des objets (instances) qui ne seront créés qu'une seule fois grâce à Singleton
    //NB:avec cette synthaxe (...*) on est obligé d'utiliser -> dans le main
    //on pourrait plutôt utiliser ... & ... (on utilisera . dans le main)
    MotionControlSystem* motionControlSystem = &MotionControlSystem::Instance();
    // motionControlSystem est tout simplement un pointeur vers une r�f�rence d'un objet de type MotionControlSystem #TRIVIAL #USELESS

    motionControlSystem->init(); //initialise asservissement, PWM et counters
    ActuatorsMgr* actuatorsMgr = &ActuatorsMgr::Instance(); //ax12
    SensorMgr* sensorMgr = &SensorMgr::Instance(); //capteurs, contacteurs, jumper
    Voltage_controller* voltage = &Voltage_controller::Instance();//contrôle batterie Lipos

    ElevatorMgr* elevatorMgr = &ElevatorMgr::Instance();

    //INITIALISATION DES ACTIONNEURS:
    actuatorsMgr->braPelReleve();
    actuatorsMgr->pellePosDeplacement();
    actuatorsMgr->largueRepos();
    actuatorsMgr->moduleMid(0);
    actuatorsMgr->moduleMid(1);
    actuatorsMgr->caleMidD();
    actuatorsMgr->caleMidG();

    volatile int32_t usSendDelay=Millis();

    char order[64]; //Permet le stockage du message re�u par la liaison s�rie

    volatile bool translation = true;        //permet de basculer entre les r�glages de cte d'asserv en translation et en rotation
    volatile bool verificationOrder= false;  //A mettre à true pour gérer le timeout de la com' haut niveau(bloque screen, sauf si vous êtes très rapide)
    volatile bool autoUpdateUS = false;      //Envoie les valeurs des capteurs au HL après les avoir mises à jour
    volatile bool isTimeout=false;
    while(1)
    {
        sensorMgr->refresh(motionControlSystem->getMovingDirection()); //les capteurs envoient un signal de durée 10 ms devant eux

        if(autoUpdateUS && Millis()-usSendDelay > 100) {
            serial.printflnUS("%d", sensorMgr->getSensorDistanceAVG());
            serial.printflnUS("%d", sensorMgr->getSensorDistanceAVD());
            serial.printflnUS("%d", sensorMgr->getSensorDistanceARG());
            serial.printflnUS("%d", sensorMgr->getSensorDistanceARD());
            usSendDelay=Millis();
        }

        uint8_t tailleBuffer = serial.available(); //taille utilisée pour le passage des données dans le câble série

        if (tailleBuffer && tailleBuffer < RX_BUFFER_SIZE - 1) //s'il reste de la place dans le câble série
        {
            isTimeout=serial.read(order);
            if(!isTimeout){
                continue;
            }

            serial.printfln("_");				//Acquittement

            serial.printflnDebug("Ordre reçu et acquitté: %s", order);
/*			 __________________
 * 		   *|                  |*
 *		   *|  TESTS DE BASE   |*
 *		   *|__________________|*
 */

            if(!strcmp("?",order))				//Ping
            {
                serial.printfln("0");
                serial.printflnDebug("PING");
            }
            else if(!strcmp("f",order))			//Indiquer l'�tat du mouvement du robot
            {
                serial.printfln("%d", motionControlSystem->isMoving());			//Robot en mouvement ou pas ?
                serial.printfln("%d", motionControlSystem->isMoveAbnormal());	//Cet �tat du mouvement est il anormal ?
            }
            else if(!strcmp("?xyo",order))		//Indiquer la position du robot (en mm et radians)
            {
                serial.printfln("%f", motionControlSystem->getX());
                serial.printfln("%f", motionControlSystem->getY());
                serial.printfln("%f", motionControlSystem->getAngleRadian());
            }
            else if(!strcmp("d", order))		//Ordre de d�placement rectiligne (en mm)
            {
                int deplacement = 0;
                serial.read(deplacement);
                serial.printfln("_");           //Acquittement
                serial.printflnDebug("Translation %d", deplacement);
                motionControlSystem->orderTranslation(deplacement);
                serial.printflnDebug("Translation commencée");

            }
                /*
            else if(!strcmp("dtest",order))
            {
                int distance = 0;
                serial.printfln("Distance du test : (mm)");
                serial.read(distance);
                motionControlSystem->distanceTest = distance;
            } */
            else if(!strcmp("t", order))		//Ordre de rotation via un angle absolu (en radians)
            {
                float angle = motionControlSystem->getAngleRadian();
                serial.read(angle);
                serial.printfln("_");           //Acquittement
                serial.printflnDebug("Rotation vers %f", angle);
                motionControlSystem->orderRotation(angle, MotionControlSystem::FREE);
                serial.printflnDebug("Rotation commencée");
            }
            else if(!strcmp("t3", order))		//Ordre de rotation via un angle relatif (en radians)
            {
                float angle_actuel = motionControlSystem->getAngleRadian(), delta_angle = 0;
                serial.read(delta_angle);
                serial.printfln("_");
                motionControlSystem->orderRotation(angle_actuel + delta_angle, MotionControlSystem::FREE);
            }
            else if(!strcmp("r", order))		//Ordre de rotation via un angle relatif (en degr�s)
            {
                float angle_actuel = motionControlSystem->getAngleRadian()*180/PI, delta_angle = 0;
                serial.read(delta_angle);
                serial.printfln("_");
                motionControlSystem->orderRotation((angle_actuel + delta_angle)*PI/180, MotionControlSystem::FREE);
            }
                /*else if(!strcmp("dc", order)) //Rotation + translation = trajectoire courbe !
                {
                    float arcLenght = 0;
                    float curveRadius = 0;
                    serial.read(arcLenght);
                    serial.printfln("_");		//Acquittement
                    serial.read(curveRadius);
                    serial.printfln("_");		//Acquittement
                    motionControlSystem->orderCurveTrajectory(arcLenght, curveRadius);
                }
              */
            else if(!strcmp("stop",order))      //Ordre d'arr�t (asservissement � la position actuelle)
            {
                motionControlSystem->stop();
            }
            else if(!strcmp("dts",order))       //définir le Delay To Stop (temps à l'arrêt avant de considérer un blocage)
            {
                uint32_t delayToStop = 0;
                serial.printflnDebug("Delay to stop ? (ms)");
                serial.read(delayToStop);
                motionControlSystem->setDelayToStop(delayToStop);
                serial.printflnDebug("Delay to stop = %d", delayToStop);
            }


/*			 _____________________
 * 		   *|                     |*
 *		   *|    AUTRES TESTS     |*
 *		   *|_____________________|*
 */

            else if(!strcmp("cx",order))		//R�gler la composante x de la position (en mm)
            {
                float x;
                serial.read(x);
                serial.printfln("_");//Acquittement
                motionControlSystem->setX(x);
            }
            else if(!strcmp("cy",order))		//R�gler la composante y de la position (en mm)
            {
                float y;
                serial.read(y);
                serial.printfln("_");//Acquittement
                motionControlSystem->setY(y);
            }
            else if(!strcmp("co",order))		//R�gler l'orientation du robot (en radians)
            {
                float o;
                serial.read(o);
                serial.printfln("_");//Acquittement
                motionControlSystem->setOriginalAngle(o);
            }
            else if(!strcmp("ctv",order))       //R�gler la vitesse de translation
            {
                float speed = 0;                // unit� de speed : mm/s
                serial.read(speed);
                serial.printfln("_");
                motionControlSystem->setTranslationSpeed(speed);
            }
            else if(!strcmp("crv", order))     //R�gler la vitesse de rotation
            {
                float speedRotation = 0;       // rad/s
                serial.read(speedRotation);
                serial.printfln("_");
                motionControlSystem->setRotationSpeed(speedRotation);
            }

            else if(!strcmp("efm", order)) // Activer les mouvements forc�s (sans blocage)
            {
                motionControlSystem->enableForcedMovement();
            }

            else if(!strcmp("dfm", order)) // d�sactive le for�age
            {
                motionControlSystem->disableForcedMovement();
            }
            else if(!strcmp("auto", order))
            {
                autoUpdatePosition = !autoUpdatePosition; //active ou désactive l'envoi automatique de position au HL
            }
            else if(!strcmp("sus", order))
            {
                autoUpdateUS=!autoUpdateUS;
            }
            else if(!strcmp("read0", order)){
                verificationOrder=!verificationOrder;
            }

/*			 _____________________
 * 		   *|                     |*
 *		   *|   ASSERVISSEMENTS   |*
 *		   *|_____________________|*
 */

            else if(!strcmp("ct0",order))		//D�sactiver l'asservissement en translation
            {
                motionControlSystem->enableTranslationControl(false);
                serial.printflnDebug("non asservi en translation");
            }
            else if(!strcmp("ct1",order))		//Activer l'asservissement en translation
            {
                motionControlSystem->enableTranslationControl(true);
                serial.printflnDebug("asservi en translation");
            }
            else if(!strcmp("cr0",order))		//D�sactiver l'asservissement en rotation
            {
                motionControlSystem->enableRotationControl(false);
                serial.printflnDebug("non asservi en rotation");
            }
            else if(!strcmp("cr1",order))		//Activer l'asservissement en rotation
            {
                motionControlSystem->enableRotationControl(true);
                serial.printflnDebug("asservi en rotation");
            }
            else if(!strcmp("cv0",order))		//Désactiver l'asservissement en vitesse
            {
                motionControlSystem->enableSpeedControl(false);
                serial.printflnDebug("non asservi en vitesse");
            }
            else if(!strcmp("cv1",order))		//Activer l'asservissement en vitesse
            {
                motionControlSystem->enableSpeedControl(true);
                serial.printflnDebug("asservi en vitesse");
            }


/*			 ___________________________
 * 		   *|                           |*
 *		   *|         MONTLHERY         |*
 *		   *|   DEPLACEMENT ET ROTATION |*
 *		   *|    AVEC ASSERVISSEMENT    |*
 *		   *|___________________________|*
 */

            else if(!strcmp("montlhery", order))                        // Désactive l'asservissement en translation et en rotation
            {
                motionControlSystem->enableTranslationControl(false);
                motionControlSystem->enableRotationControl(false);
            }

            else if(!strcmp("av", order))
            {
                motionControlSystem->setRawPositiveTranslationSpeed();  // definit la consigne max de vitesse de translation envoi�e au PID (trap�ze)
                // déplacement vers l'avant avec asservissement
            }

            else if(!strcmp("rc", order))
            {
                motionControlSystem->setRawNegativeTranslationSpeed();  // definit la consigne max de vitesse de translation envoi�e au PID (trap�ze)
                // déplacement vers l'arrière avec asservissement
            }

            else if(!strcmp("td", order))
            {
                motionControlSystem->setRawNegativeRotationSpeed();     // definit la consigne max de vitesse de rotation envoi�e au PID (trap�ze)
                // rotation sens antitrigo avec asservissement
            }

            else if(!strcmp("tg", order))
            {
                motionControlSystem->setRawPositiveRotationSpeed();     // definit la consigne max de vitesse de rotation envoi�e au PID (trap�ze)
                // rotation sens antitrigo avec asservissement
            }

            else if(!strcmp("sstop", order))                            // Stoppe le robot
            {
                motionControlSystem->setRawNullSpeed();
            }


/*			 _________________________________
 * 		   *|                                 |*
 *		   *|CONSTANTES D'ASSERV (pour le PID)|*
 *		   *|_________________________________|*
 */

            else if(!strcmp("toggle",order))
            {
                translation = !translation;   //Bascule entre le réglage d'asserv en translation et en rotation
                if(translation)
                    serial.printflnDebug("reglage de la transation");
                else
                    serial.printflnDebug("reglage de la rotation");
            }
            else if(!strcmp("display",order)) //affiche les paramètres des PID des différentes asserv (translation, rotation, vitesse à droite, vitesse à gauche)
            {
                float
                        kp_t, ki_t, kd_t,	  // Translation
                        kp_r, ki_r, kd_r,	  // Rotation
                        kp_g, ki_g, kd_g,	  // Vitesse gauche
                        kp_d, ki_d, kd_d;	  // Vitesse droite
                motionControlSystem->getTranslationTunings(kp_t, ki_t, kd_t);
                motionControlSystem->getRotationTunings(kp_r, ki_r, kd_r);
                motionControlSystem->getLeftSpeedTunings(kp_g, ki_g, kd_g);
                motionControlSystem->getRightSpeedTunings(kp_d, ki_d, kd_d);
                serial.printflnDebug("trans : kp= %g ; ki= %g ; kd= %g", kp_t, ki_t, kd_t);
                serial.printflnDebug("rot   : kp= %g ; ki= %g ; kd= %g", kp_r, ki_r, kd_r);
                serial.printflnDebug("gauche: kp= %g ; ki= %g ; kd= %g", kp_g, ki_g, kd_g);
                serial.printflnDebug("droite: kp= %g ; ki= %g ; kd= %g", kp_d, ki_d, kd_d);
            }
                /*else if(!strcmp("autoasserv" ,order))// Commande pour le programme d'autoasserv (python)
                {
                    float
                        kp_g, kp_d, ki_g, ki_d, kd_g, kd_d;

                    motionControlSystem->getLeftSpeedTunings(kp_g, ki_g, kd_g);
                    motionControlSystem->getRightSpeedTunings(kp_d, ki_d, kd_d);

                    motionControlSystem->printTracking();
                    serial.printf("endtest");
                }
                 */

                // ***********  Paramètres du PID pour l'asserv en position (TRANSLATION)  ***********
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

                // ***********  Paramètres du PID pour l'asserv en ROTATION  ***********
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

                // ***********  Paramètres du PID pour l'asserv en vitesse à gauche  ***********
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

                // ***********  Paramètres du PID pour l'asserv en vitesse à droite ****************
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


/*			 __________________
 * 		   *|                  |*
 *		   *|     CAPTEURS     |*
 *		   *|__________________|*
 */

            else if(!strcmp("usavd",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceARD());//en mm
            }
            else if(!strcmp("usavg",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceARG());//en mm
            }
            else if(!strcmp("usard",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceAVD());//en mm
            }
            else if(!strcmp("usarg",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceAVG());//en mm
            }
            else if(!strcmp("capteurs",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("AVG %d", sensorMgr->getSensorDistanceAVG());//en mm
                serial.printfln("AVD %d", sensorMgr->getSensorDistanceAVD());//en mm
                serial.printfln("ARG %d", sensorMgr->getSensorDistanceARG());//en mm
                serial.printfln("ARD %d", sensorMgr->getSensorDistanceARD());//en mm
            }
                /*
                else if(!strcmp("usard",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceARD());//en mm
            }
            else if(!strcmp("usarg",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceARG());//en mm
            }
            else if(!strcmp("usavd",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceAVD());//en mm
            }
            else if(!strcmp("usavg",order))		//Indiquer la distance mesur�e par les capteurs � ultrason
            {
                serial.printfln("%d", sensorMgr->getSensorDistanceAVG());//en mm
            }
    */

/*			 _____________________
 * 		   *|                     |*
 *		   *|CONTACTEURS ET JUMPER|*
 *		   *|_____________________|*
 */

            else if(!strcmp("j",order))			    //Indiquer l'�tat du jumper (0='en place'; 1='dehors')
            {
                serial.printfln("%d", sensorMgr->isJumperOut());
            }
            else if(!strcmp("c1",order))			//Indiquer l'�tat du contacteur1 (0='non appuyé'; 1='appuyé')
            {
                serial.printfln("%d", sensorMgr->isContactor1engaged());
            }
            else if(!strcmp("c2",order))			//Indiquer l'�tat du contacteur2 (0='non appuyé'; 1='appuyé')
            {
                serial.printfln("%d", sensorMgr->isContactor2engaged());
            }
            else if(!strcmp("c3",order))			//Indiquer l'�tat du contacteur3 (0='non appuyé'; 1='appuyé')
            {
                serial.printfln("%d", sensorMgr->isContactor3engaged());
            }


/*			 __________________
 * 		   *|                  |*
 *		   *|	ACTIONNEURS	   |*
 *		   *|__________________|*
 */

                /* --- AX12 ---*/
                //Gestion des ID des AX12
            else if(!strcmp("setallid",order))         //permet de donner les ids définis aux ax12
            {
                actuatorsMgr->setAllID();
            }
            else if(!strcmp("setpelleid",order))       //permet de donner les ids aux ax12 de la pelle
            {
                actuatorsMgr->setPelleID();
            }
            else if(!strcmp("setmoduleid",order))      //permet de donner les ids aux ax12 de l'attrappe module
            {
                actuatorsMgr->setModuleID();
            }

                //gestion des paramètres
            else if (!strcmp("changeangleax12",order)) //permet de modifier les angles min et max de l'ax12 de test
            {
                uint16_t anglemin=0,anglemax=1023;
                serial.printfln("Entrez l'angle minimal");
                serial.read(anglemin);
                serial.printfln("Entrez l'angle max");
                serial.read(anglemax);
                actuatorsMgr->changeangle(anglemin,anglemax);
            }
            else if(!strcmp("caxs", order)) {          //modifie la vitesse de l'ax12 de test
                int speed = 100;
                serial.printfln("Entrez vitesse");
                serial.read(speed);
                actuatorsMgr->changeAXSpeed(speed);
                serial.printfln("Done");
            }
                //pour avoir des mouvements plus fluides on augmente les slopes et on diminue le punch
            else if(!strcmp("setPunch", order))
            {
                actuatorsMgr->setPunch();              // couple minimal appliqué
            }
            else if(!strcmp("setSlopes", order))
            {
                actuatorsMgr->setSlopes(); // longeur de la pente du couple appliqué en f(distance à la position voulue)
                // (on peut donc modifier la pente de la courbe du couple appliqué)

            }

                //ax de test:
            else if (!strcmp("testax12",order)) {     //permet de faire bouger l'ax12 de test
                uint16_t pos = 0;
                serial.printfln("Entrez la position");
                serial.read(pos);
                actuatorsMgr->setAXpos(pos);
                serial.printfln("Done");
            }
            else if (!strcmp("reanimation",order))    //permet de réanimer certains ax12
            {
                actuatorsMgr->reanimation();
            }

            else if(!strcmp("synctest", order))
            {
                actuatorsMgr->testSync1();
            }


/*			 ____________________
 * 		   *|                    |*
 *		   *|    Pelle T-3000    |*
 *		   *|____________________|*
 */

            else if (!strcmp("bpr",order)) //(releve le bras de la pelleteuse)
            {
                actuatorsMgr->braPelReleve();
            }
            else if (!strcmp("bpd",order)) //(abaisse le bras de la pelleteuse)
            {
                actuatorsMgr->braPelDeplie();
            }
            else if (!strcmp("bpra",order)) //(abaisse un peu plus le bras de la pelleteuse)
            {
                actuatorsMgr->braPelRam();
            }
            else if (!strcmp("bpm", order)) // (position intermédiaire des bras de pelleteuse)
            {
                actuatorsMgr->braPelMoit();
            }
            else if (!strcmp("pd", order))
            {
                actuatorsMgr->pelleInit();  //position pré prise de boules de la pelle
            }
            else if (!strcmp("pm", order))
            {
                actuatorsMgr->pelleMoit();  //position post prise de boules de la pelle
            }
            else if (!strcmp("pt", order))
            {
                actuatorsMgr->pelleTient(); //position pour tenir les boules en haut
            }
            else if (!strcmp("pf", order))
            {
                actuatorsMgr->pelleLib();   //position de livraison de boules de la pelle
            }
            else if(!strcmp("pelreasserv", order))
            {
                actuatorsMgr->pelreasserv();
            }
            else if(!strcmp("pb", order))
            {
                actuatorsMgr->pellePosDeplacement();
            }


/*			 _____________________
 * 		   *|                     |*
 *		   *|Attrappes Module SSV2|*
 *		   *|_____________________|*
 */

                //0 = droit, 1 = gauche
                //Côté droit
            else if (!strcmp("amdd", order))
            {
                actuatorsMgr->moduleDeb(0); //position derrière
            }
            else if (!strcmp("ammd", order))
            {
                actuatorsMgr->moduleMid(0);
            }
            else if (!strcmp("amfd", order))
            {
                actuatorsMgr->moduleFin(0); //ramène le module
            }

                //Côté gauche
            else if (!strcmp("amdg", order))
            {
                actuatorsMgr->moduleDeb(1); //position derrière
            }
            else if (!strcmp("ammg", order))
            {
                actuatorsMgr->moduleMid(1);
            }
            else if (!strcmp("amfg", order)) //ramène le module
            {
                actuatorsMgr->moduleFin(1);
            }


/*			 ___________________
 * 		   *|                   |*
 *		   *|   Cales Module    |*
 *		   *|___________________|*
 */

                //droit
            else if (!strcmp("cmmd", order))
            {
                actuatorsMgr->caleMidD(); //position de repos
            }
            else if (!strcmp("cmdd",order))
            {
                actuatorsMgr->caleHautD(); //avant de pousser le module
            }
            else if(!strcmp("cmfd",order))
            {
                actuatorsMgr->caleBasD();  //pour pousser le module
            }

                //gauche
            else if(!strcmp("cmmg", order))
            {
                actuatorsMgr->caleMidG(); //position de repos
            }
            else if (!strcmp("cmdg",order))
            {
                actuatorsMgr->caleHautG(); //avant de pousser le module
            }
            else if(!strcmp("cmfg",order))
            {
                actuatorsMgr->caleBasG();  //pour pousser le module
            }


/*			 ___________________
 * 		   *|                   |*
 *		   *|   Largue Module   |*
 *		   *|___________________|*
 */

            else if(!strcmp("lmd",order))
            {
                actuatorsMgr->largueRepos();  //position derrière
            }
            else if(!strcmp("lmf",order))
            {
                actuatorsMgr->larguePousse(); //position devant (largue les modules)
            }
            else if(!strcmp("lmfl",order))
            {
                actuatorsMgr->larguePousselent(); //position devant (largue les modules)
            }


/*		     ___________________
* 		   *|                   |*
*		   *|     Ascenseur     |*
*		   *|___________________|*
*/

            else if(!strcmp("ascdown", order))
            {
                elevatorMgr->moveTo(ElevatorMgr::DOWN);
            }
            else if(!strcmp("ascup", order))
            {
                elevatorMgr->moveTo(ElevatorMgr::UP);
            }
            else if(!strcmp("ascstop", order))
            {
                elevatorMgr->stop();
            }
            else if(!strcmp("ascpwm", order))
            {
                int pwm=0;
                serial.printflnDebug("Entrez le pwm à utiliser( [0;10]");
                serial.read(pwm);
                elevatorMgr->setPWM(pwm);
                serial.printflnDebug("done");
            }
            else if(!strcmp("ascstatus", order))
            {
                elevatorMgr->getData();
            }

/*			 ________________________
 * 		   *|                        |*
 *		   *|COMMANDES/TESTS DE DEBUG|*
 *		   *|________________________|*
*/
/*
            else if(!strcmp("rawpwm", order)) {
                motionControlSystem->enable(false);
                int16_t pwm;
                int sens;
                serial.printflnDebug("Entrer le pwm:");
                serial.read(pwm);
                serial.printflnDebug("Entrer roue: 0=gauche, 1=droite");
                serial.read(sens);
                if (sens == 0) {
                    motionControlSystem->orderRawPwm(Side::LEFT, pwm);
                    serial.printflnDebug("pwm dans le moteur: %d", motionControlSystem->getMotorPWM(0));
                    motionControlSystem->getSens(0);
                } else if (sens == 1) {
                    motionControlSystem->orderRawPwm(Side::RIGHT, pwm);
                    serial.printflnDebug("pwm dans le moteur: %d", motionControlSystem->getMotorPWM(1));
                    motionControlSystem->getSens(1);
                }


            }
            */
            else if(!strcmp("pfdebug", order))
            {
                serial.printfln("%d", (int) Counter::getLeftValue());
                serial.printfln("%d", (int) Counter::getRightValue());
                serial.printfln("%f", motionControlSystem->getAngleRadian());
                serial.printfln("%d",  (int) motionControlSystem->getLeftSpeed().value());
                serial.printfln("%d",  (int) motionControlSystem->getRightSpeed().value());
                serial.printfln("%d", (int) motionControlSystem->getTranslationSetPoint());
                serial.printfln("%d", (int) motionControlSystem->getLeftSetPoint());
                serial.printfln("%d", (int) motionControlSystem->getRightSetPoint());
            }
            else if(!strcmp("asserdata", order))
            {
                serial.printflnDebug("X: %d -- Y: %d -- O: %f",
                                     (int) motionControlSystem->getX(),
                                     (int) motionControlSystem->getY(), motionControlSystem->getAngleRadian());
                serial.printflnDebug("Vitesse Gauche:%d",
                                     (int) motionControlSystem->getLeftSpeed().value());
                serial.printflnDebug("Vitesse Droite:%d",
                                     (int) motionControlSystem->getRightSpeed().value());
                serial.printflnDebug("Consignes : Transl: %d --- Vit.G: %d --- Vit.D: %d",
                                     (int) motionControlSystem->getTranslationSetPoint(),
                                     (int) motionControlSystem->getLeftSetPoint(),
                                     (int) motionControlSystem->getRightSetPoint());
                serial.printflnDebug("codeuse gauche--droite: %d --    %d",
                                     motionControlSystem->leftTicks, motionControlSystem->rightTicks);
                //  motionControlSystem->getData();

            }

            else if(!strcmp("rp",order))             //Reset position et angle du robot, et le stoppe
            {
                motionControlSystem->resetPosition();
                serial.printfln("Reset position");
            }
            else if(!strcmp("testSpeed",order))      //n'active que l'asserv en vitesse et fait un test (déplacement avec une certaine vitesse) avec tracking
            {
                motionControlSystem->testSpeed();
            }
            else if(!strcmp("continualTest",order))  //Test long
            {
                motionControlSystem->longTestSpeed();
            }
            else if(!strcmp("testPosition",order))   //active toutes les asserv et fait un test (déplacement) avec tracking
            {
                motionControlSystem->testPosition();
            }
            else if(!strcmp("testRotation",order))   //active toutes les asserv et fait un test (rotation) avec tracking
            {
                motionControlSystem->testRotation();
            }


/*			 _______________________
 * 		   *|                       |*
 *		   *| COMMANDES DE TRACKING |*
 *		   *|    DES VARIABLES DU   |*
 *		   *|     SYSTEME (DEBUG)   |*
 *		   *|_______________________|*
 */

            else if(!strcmp("trackAll",order))
            {
                motionControlSystem->printTrackingAll();  //affiche les paramètres et valeurs (vitesse,...) pour permettre le débug
            }

            else if(!strcmp("adc", order))
            {
                serial.printfln("%d", voltage->test());   // Pour tester la tension d'alimentation selon l'adc
            }


/*			 __________________
 * 		   *|                  |*
 *		   *|  ERREURS DE COM  |*
 *		   *|__________________|*
 */

            else if(!strcmp("uoe",order))
                // test d'un mauvais retour bas niveau --> haut niveau
            { // test pour faire exprès d'envoyer n'importe quoi au HL
                serial.printfln("Une fraise");
            }
            else // Sinon, Ordre inconnu
            {
                serial.printfln("Ordre inconnu");
            }


        }
#if DEBUG
        else if(tailleBuffer == RX_BUFFER_SIZE - 1)
            //si l'espace utilisé dans le câble série est égal à la taille totale du buffer - 1
        {
            serial.printfln("CRITICAL OVERFLOW !");
            motionControlSystem->enableTranslationControl(false);
            motionControlSystem->enableRotationControl(false);
            motionControlSystem->enableSpeedControl(false);

            while(true);
        }
#endif
    }
}
//fin main


/*			 ___________________
 * 		   *|                   |*
 *		   *|   INTERRUPTIONS   |*
 *		   *|___________________|*
 */

extern "C" { //indique au compilateur que les fonctions créées sont en C et non en C++
//Interruptions sur le TIMER4
void TIM4_IRQHandler(void) { //2kHz = 0.0005s = 0.5ms
    volatile static uint32_t i = 0, j = 0, k = 0, l = 0; //compteurs pour lancer des méthodes à différents moments
    static MotionControlSystem *motionControlSystem = &MotionControlSystem::Instance();
    static Voltage_controller *voltage = &Voltage_controller::Instance();
    static ElevatorMgr *elevatorMgr = &ElevatorMgr::Instance();
    //static ElevatorMgr &elevatorMgr = ElevatorMgr::Instance();

    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) { //arbalète
        //TIM_GetITStatus vérifie si l'interruption a eu lieu (SET) ou non (RESET)
        //SET donc remise à 0 manuelle du flag d'interruption nécessaire
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update); //remet les bits de l'interruption à 0

        //Asservissement et mise à jour de la position
        motionControlSystem->control();
        motionControlSystem->updatePosition();
        //elevatorMgr.elevatorControl();


        if (j >= 5) { //0.5ms x 5 = 2.5ms
            motionControlSystem->track(); //stocke les valeurs de débug
            motionControlSystem->manageStop(); //regarde si le robot bouge normalement

            j = 0;
        }

        if (k >= 2000) {

            voltage->measure(); //regarde la batterie des Lipos
            k = 0;
        }

        if (l >= 200) {
            elevatorMgr->control();
            if (autoUpdatePosition && !serial.available()) {
                //si l'envoi automatique de position au HL est activé et que la série a de la place diponible
                //on affiche la position et l'angle du robot
                serial.printflnPosition("%d", (int) motionControlSystem->getX());
                serial.printflnPosition("%d", (int) motionControlSystem->getY());
                serial.printflnPosition("%f", motionControlSystem->getAngleRadian());
                serial.printflnPosition("%d", (int) motionControlSystem->getLeftSpeed().value());
                serial.printflnPosition("%d", (int) motionControlSystem->getRightSpeed().value());
                serial.printflnPosition("%d", (int) motionControlSystem->getTranslationSetPoint());
                serial.printflnPosition("%d", (int) motionControlSystem->getLeftSetPoint());
                serial.printflnPosition("%d", (int) motionControlSystem->getRightSetPoint());
            } else if (autoUpdatePosition) {
                serial.printflnDebug("Serie occupee !!!");
                serial.printflnDebug("available = %d", serial.available());
            }


            l = 0;
        }

        k++;
        i++;
        j++;
        l++;
    }
}

/*
void EXTI1_IRQHandler(void) // interruptions sur pins
 {
 static SensorMgr* sensorMgr = &SensorMgr::Instance();  // Capteurs US

 //Interruptions des capteurs US : pour calculer la distance à l'objet
 if (EXTI_GetITStatus(EXTI_Line1) != RESET) {           // arbalète
           		sensorMgr->sensorInterrupt(2);          // on lance l'interruption qui calcule la distance à l'objet
        		EXTI_ClearITPendingBit(EXTI_Line1);     // Clear interrupt flag : on passe de SET à RESET
        		                                        // le passge de RESET à SET est interne à la carte
 }
*/


void EXTI0_IRQHandler(void){
    static SensorMgr *sensorMgr = &SensorMgr::Instance();
    if (EXTI_GetITStatus(EXTI_Line0) != RESET){
        sensorMgr->sensorInterrupt(1);          //AVG
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}


void EXTI15_10_IRQHandler(void) {
    static SensorMgr *sensorMgr = &SensorMgr::Instance();

    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {

        sensorMgr->sensorInterrupt(2);          //ARD
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if(EXTI_GetITStatus(EXTI_Line14) != RESET){ //AVD
        sensorMgr->sensorInterrupt(0);
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET) {

        sensorMgr->sensorInterrupt(3);          //ARG
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}
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

