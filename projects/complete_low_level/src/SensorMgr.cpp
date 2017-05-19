#include <stm32f4xx_gpio.h>
#include "SensorMgr.h"


/**
 * Gestion des capteurs !
 * @author ?(dénoncez vous ;)), rémi, melanie
 **/


/*		PINS DES CAPTEURS
 *
 * 	CONTACTEURS:
 * 		Jumper : PC10
 * 		Contacteur 1	: PB12
 * 		Contacteur 2	: PC12
 * 		Contacteur 3	: PD11
 *
 * 	ULTRASONS:
 * 		Avant Droit   (US4):	avant,PD8, maintenant, PB14
 * 		Avant Gauche  (US2):	PC0
 * 		Arri�re Droit (US3):	PC13
 * 		Arri�re Gauche (US1):	PC15
 */


SensorMgr::SensorMgr():

// On définit des capteurs US qui attendent le prochain front montant
        ultrasonAVD(),
        ultrasonAVG(),
        ultrasonARD(),
        ultrasonARG()

{
    refreshDelay = 25;   // temps tout les combien les capteurs envoient leur signal de 10ms devant eux
    lastRefreshTime = Millis(); // on initialise le moment de dernier envoi de ce signal


/*       ___________________________
       *|				            |*
       *|  Initialisation des ports |*
       *|___________________________|*
*/

    // Définition des variables qu'on va initialiser
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // Initialisation des ports GPIO utilisés pour communiquer au niveau des pins
    GPIO_StructInit(&GPIO_InitStruct); //Initialise avec les valeurs par d�faut

    // Activation des horloges pour les différents ports GPIO
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //Active l'horloge du port A
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //Active l'horloge du port B
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //Active l'horloge du port C
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); //Active l'horloge du port D


/*     ______________________________________
	 *|								         |*
	 *| Initialisation Jumper et contacteurs |*
	 *|______________________________________|*
*/

    // Initialisation de la pin du Jumper (PC10)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;         // le canal des pins de numéro 10 est initialisé
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;       // mode input
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;   // rien de particulier
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz; // Fréquence maximale en output (100MHz est très élevé)
    GPIO_Init(GPIOC, &GPIO_InitStruct);             // on actualise les paramètres du port GPIOC

    // Contacteur 1 (PB12)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Contacteur 2 (PC12)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Contacteur 3 (PD11)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);


/*     _______________________________________________________
	 *|								                          |*
	 *| Initialisation des interruptions pour les capteurs US |*
	 *|_______________________________________________________|*
*/
    // Capteur US ARD : PC13

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // active l'horloge de SYSCFG (System Configuration)

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;              // mode input
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;            // le type en output est Push-Pull
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;                 // le canal des pins de numéro 6 est initialisé
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;            // si le signal est indéterminé, il sera de potentiel 0
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;        // fréquence maximale en output
    GPIO_Init(GPIOC, &GPIO_InitStruct);                    // actualisation des paramètres du port GPIOA

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); // définit le numéro de canal (6) par lequel le port A va lancer l'interruption

    EXTI_InitStruct.EXTI_Line = EXTI_Line13;                       // on connecte PA6 à l'EXTI_Line6
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;                        // autorise l'interruption
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;              // on passe dans le mode d'interruptions
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;           // l'interruption se déclenche pour un front montant reçu
    EXTI_Init(&EXTI_InitStruct);                                  // actualisation des paramètres de l'EXTI_Line

    // NVIC = Nested Vectored Interrupt Controller
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;               // vecteur d'interruption associé à l'EXTI_Line utilisée
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;     // priorité de ce vecteur
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;            // sous-priorité de ce vecteur (en cas de même priorité avec un autre vecteur)
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;                  // autorise l'interruption
    NVIC_Init(&NVIC_InitStruct);                                  // actualisation des paramètres de NVIC

    ultrasonARD.init(GPIOC, GPIO_InitStruct, EXTI_InitStruct);    // on actualise le capteur US avec le port, la pin et le canal définis ci-dessus

    // Capteur US ARG : PC15

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource15);

    EXTI_InitStruct.EXTI_Line = EXTI_Line15;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonARG.init(GPIOC, GPIO_InitStruct, EXTI_InitStruct);

    // Capteur US AVD : avant, PD8, maitenant PB14

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);

    EXTI_InitStruct.EXTI_Line = EXTI_Line14;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonAVD.init(GPIOB, GPIO_InitStruct, EXTI_InitStruct);

    // Capteur US AVG : PC0

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);

    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonAVG.init(GPIOC, GPIO_InitStruct, EXTI_InitStruct);
}


/*     __________________________________________________________________
	 *|								                                     |*
	 *| Fonctions de r�cup�ration de l'�tat du jumper et des contacteurs |*
	 *|__________________________________________________________________|*
*/

bool SensorMgr::isJumperOut() const{
    return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10);
    // 0="en place", 1="retiré"
}
bool SensorMgr::isContactor1engaged() const{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
    // 0="non appuyé", 1="appuyé"
}
bool SensorMgr::isContactor2engaged() const{
    return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12);
}
bool SensorMgr::isContactor3engaged() const{
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11);
}


/*     ___________________________________________
	 *|								              |*
	 *| Fonctions d'actualisation des capteurs US |*
	 *|___________________________________________|*
*/

void SensorMgr::refresh(MOVING_DIRECTION direction) // les capteurs envoient un signal de durée 10 ms devant eux
{
    currentTime = Millis(); // current time (en s)
    if(currentTime - lastRefreshTime >= refreshDelay && direction == FORWARD) // actualisation toutes les 13ms
    {
        ultrasonAVD.refresh();
        ultrasonAVG.refresh();
        lastRefreshTime=Millis();
    }

    else if(currentTime - lastRefreshTime >= refreshDelay && direction == BACKWARD)
    {
        ultrasonARD.refresh();
        ultrasonARG.refresh();
        lastRefreshTime=Millis();
    }
    else if(currentTime - lastRefreshTime >= refreshDelay && direction == NONE)
    {
        ultrasonAVD.refresh();
        ultrasonAVG.refresh();
        ultrasonARD.refresh();
        ultrasonARG.refresh();
        lastRefreshTime=Millis();
    }

}


/*     __________________________________________
	 *|								             |*
	 *| Fonctions d'interruption des capteurs US |*
	 *|__________________________________________|*
*/

void SensorMgr::sensorInterrupt(int idsensor){
    // Calcule la distance en mm par rapport au prochain objet devant soi, pour tous les capteurs
    // 0 correspond à avant gauche, 1 à avant droit, 2 à arrière gauche, 3 à arrière droit
    if(idsensor == 0) {
        ultrasonAVD.interruption();
    }
    else if (idsensor == 1){
        ultrasonAVG.interruption();
    }
    else if (idsensor == 2) {
        ultrasonARD.interruption();
    }
    else if (idsensor == 3) {
        ultrasonARG.interruption();
    }
}


/*     ______________________________________________________________________
	 *|								                                         |*
	 *| Fonctions de r�cup�ration de la distance mesur�e par les capteurs US |*
	 *|______________________________________________________________________|*
*/

int SensorMgr::getSensorDistanceAVD() {
    return ultrasonAVD.value();
    // la valeur retournée est celle calculée dans l'interruption
}
int SensorMgr::getSensorDistanceAVG() {
    return ultrasonAVG.value();
}
int SensorMgr::getSensorDistanceARD() {
    return ultrasonARD.value();
}
int SensorMgr::getSensorDistanceARG() {
    return ultrasonARG.value();
}