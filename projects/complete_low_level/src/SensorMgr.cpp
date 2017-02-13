#include <stm32f4xx_gpio.h>
#include "SensorMgr.h"

/*		PINS DES CAPTEURS
 *
 * 	ULTRASONS:
 * 		Avant Droit   :	PA6
 * 		Avant Gauche  :	PA4
 * 		Arri�re Droit :	PB1
 * 		Arri�re Gauche:	PA7
 *
 * 	CONTACTEURS:
 * 		Jumper : PC10
 * 		Contacteur 1	: PC0
 * 		Contacteur 2	: PC12
 * 		Contacteur 3	: PD11
 */


SensorMgr::SensorMgr():
	ultrasonARD(),
    ultrasonARG(),
    ultrasonAVD(),
    ultrasonAVG()

{
	lastRefreshTime = 0;
	refreshDelay = 13;//( en ms)

	/* Set variables used */
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	/*
	 * Initialisation des pins des capteurs de contact
	 */

	GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par d�faut

/*         _________________________________________
		 *|								            |*
		 *|  Capteurs de contact (portes + jumper)  |*
		 *|_________________________________________|*
*/

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//Active l'horloge du port A
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//Active l'horloge du port B
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//Active l'horloge du port C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//Active l'horloge du port D

	//Jumper (PC10)

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);



/*     ________________________________
	 *|								   |*
	 *|Initialisation des interruptions|*
	 *|________________________________|*
*/

	// Contacteur 1 (PC0)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

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



	 // Capteur US ARD : (PB1)


		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);

		EXTI_InitStruct.EXTI_Line = EXTI_Line1;
		EXTI_InitStruct.EXTI_LineCmd = DISABLE;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_Init(&EXTI_InitStruct);

		NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		ultrasonARD.init(GPIOB, GPIO_InitStruct, EXTI_InitStruct);


    //@M : à tester + voir pour les priorités


    // Capteur US ARG : (PA7)


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);

    EXTI_InitStruct.EXTI_Line = EXTI_Line7;
    EXTI_InitStruct.EXTI_LineCmd = DISABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonARD.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);

    // Capteur US AVD : (PA6)


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

    EXTI_InitStruct.EXTI_Line = EXTI_Line6;
    EXTI_InitStruct.EXTI_LineCmd = DISABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonARD.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);

    // Capteur US AVG : (PA4)


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);

    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_LineCmd = DISABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0xff;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ultrasonARD.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);





	/*
	 * Gestion des capteurs de fin de course :
	 */


}


/*
 * Fonction de mise � jour des capteurs � ultrason
 */
//@M

void SensorMgr::refresh(MOVING_DIRECTION direction)
{
	currentTime = Millis();
	static uint8_t capteur = 0;

	if(currentTime - lastRefreshTime >= refreshDelay && direction == FORWARD)
	{
		ultrasonAVD.refresh();
        ultrasonAVG.refresh();
	}

    else if(currentTime - lastRefreshTime >= refreshDelay && direction == BACKWARD)
    {
        ultrasonARD.refresh();
        ultrasonARG.refresh();
    }
    else if(currentTime - lastRefreshTime >= refreshDelay && direction == NONE)
    {
        ultrasonAVD.refresh();
        ultrasonAVG.refresh();
        ultrasonARD.refresh();
        ultrasonARG.refresh();
    }

}


/*
 * Fonctions d'interruption des capteurs � ultrason
 */

void SensorMgr::sensorInterrupt(int idsensor){

	if(idsensor == 0)
		ultrasonARD.interruption();
    //@M:à tester
    else if (idsensor == 1)
        ultrasonARG.interruption();
    else if (idsensor == 2)
        ultrasonAVD.interruption();
    else if (idsensor == 3)
        ultrasonAVG.interruption();



}
//0 correspond à avant gauche, 1 à avant droit, 2 à arrière gauche, 3 à arrière droit


void SensorMgr::ARDInterrupt(){
	ultrasonARD.interruption();
}
//@M:à tester
void SensorMgr::ARGInterrupt(){
    ultrasonARG.interruption();
}
void SensorMgr::AVDInterrupt(){
    ultrasonAVD.interruption();
}
void SensorMgr::AVGInterrupt(){
    ultrasonAVG.interruption();
}

/*
 * Fonctions de r�cup�ration de la distance mesur�e
 */

int SensorMgr::getSensorDistanceARD() {
	return ultrasonARD.value();
}
int SensorMgr::getSensorDistanceARG() {
    return ultrasonARG.value();
}
int SensorMgr::getSensorDistanceAVD() {
    return ultrasonAVD.value();
}
int SensorMgr::getSensorDistanceAVG() {
    return ultrasonAVG.value();
}



/*
 * Fonctions de r�cup�ration de l'�tat des capteurs de contact et du jumper
 */


bool SensorMgr::isJumperOut() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10);
}
//@M:à tester
bool SensorMgr::isContactor1engaged() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
}
bool SensorMgr::isContactor2engaged() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12);
}
bool SensorMgr::isContactor3engaged() const{
	return !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11);
}