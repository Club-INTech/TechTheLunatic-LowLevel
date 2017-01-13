#include <stm32f4xx_gpio.h>
#include "SensorMgr.h"

/*		PINS DES CAPTEURS
 *
 * 	ULTRASONS:
 * 		Avant Droit   :	PA6
 * 		Avant Gauche  :	PA4
 * 		Arri�re Droit :	PA7
 * 		Arri�re Gauche:	PB1
 *
 * 	CONTACTEURS:
 * 		Jumper : PC9
 * 		Contacteur 1	: PC0
 * 		Contacteur 2	: PD9
 * 		Contacteur 3	: PD11
 */


SensorMgr::SensorMgr():
	ultrasonARD()
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

	//Jumper (PC9)

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
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

	// Contacteur 2 (PD9)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	// Contacteur 3 (PD11)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);



	 // Capteur US ARD :


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
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
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
void SensorMgr::refresh(MOVING_DIRECTION direction)
{
	currentTime = Millis();
	static uint8_t capteur = 0;

	if(currentTime - lastRefreshTime >= refreshDelay)
	{
		ultrasonARD.refresh();
	}
}


/*
 * Fonctions d'interruption des capteurs � ultrason
 */

void SensorMgr::sensorInterrupt(int pin){


	if(pin == 6)
		ultrasonARD.interruption();



}


void SensorMgr::ARDInterrupt(){
	ultrasonARD.interruption();
}


/*
 * Fonctions de r�cup�ration de la distance mesur�e
 */

int SensorMgr::getSensorDistanceARD() {
	return ultrasonARD.value();
}



/*
 * Fonctions de r�cup�ration de l'�tat des capteurs de contact et du jumper
 */


bool SensorMgr::isJumperOut() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9);
}
bool SensorMgr::isContactor1engaged() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
}
bool SensorMgr::isContactor2engaged() const{
	return !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9);
}
bool SensorMgr::isContactor3engaged() const{
	return !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11);
}