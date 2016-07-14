#include "SensorMgr.h"

/*		PINS DES CAPTEURS
 *
 * 	ULTRASONS:
 * 		Avant Droit   :	PA6
 * 		Avant Gauche  :	PA4
 * 		Arrière Droit :	PA7
 * 		Arrière Gauche:	PB1
 *
 * 	CONTACTEURS:
 * 		Monte-plot		: PC15
 * 		Gobelet Droit	: PD9
 * 		Gobelet Gauche	: PD11
 */


SensorMgr::SensorMgr():
	ultrasonAVD(),
	ultrasonAVG(),
	ultrasonARG(),
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

	GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

// Desactive la clock de la SPII1 (l'accelerometre qui fout la merde sur PA5) :

	/*RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);

	SPI_Cmd(SPI1, DISABLE);
	TIM_Cmd(TIM8, DISABLE);*/



/*         _________________________________________
		 *|								            |*
		 *|  Capteurs de contact (portes + jumper)  |*
		 *|_________________________________________|*
*/

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//Active l'horloge du port C

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

	// Capteur porte DROITE OUVERTE (PC0)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Capteur porte GAUCHE FERMEE (PC1)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Capteur porte DROITE FERMEE (PC13)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Capteur porte GAUCHE OUVERTE (PC15)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);



	 // Capteur US ARD :


		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

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
	 * Capteur US AVD : PA5 ---- ATTENTION celle ci ne marche pas bien : inversion entre AVD et ARD ----
	 */

	// Activation de l'horloge du port A
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Activation de l'horloge du SYSCFG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//Réglages de la pin
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Tell system that you will use PA5 for EXTI_Line5
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);

	// PA5 is connected to EXTI_Line5
	EXTI_InitStruct.EXTI_Line = EXTI_Line5;
	// Enable interrupt
	EXTI_InitStruct.EXTI_LineCmd = DISABLE;
	// Interrupt mode
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	// Triggers on rising and falling edge
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	// Add to EXTI
	EXTI_Init(&EXTI_InitStruct);

	// Add IRQ vector to NVIC
	// PA7 is connected to EXTI_Line5, which has EXTI9_5_IRQn vector
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	// Set priority
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	// Set sub priority
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	// Enable interrupt
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	// Add to NVIC
	NVIC_Init(&NVIC_InitStruct);

	ultrasonAVD.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);


	/*
	 * Capteur US AVG : PA7
	 */

	/* Activation de l'horloge du port GPIOA */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		/* Activation de l'horloge du SYSCFG */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		/*Réglages de la pin*/
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Tell system that you will use PA7 for EXTI_Line7 */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);

		/* PA7 is connected to EXTI_Line7 */
		EXTI_InitStruct.EXTI_Line = EXTI_Line7;
		/* Enable interrupt */
		EXTI_InitStruct.EXTI_LineCmd = DISABLE;
		/* Interrupt mode */
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		/* Triggers on rising and falling edge */
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
		/* Add to EXTI */
		EXTI_Init(&EXTI_InitStruct);

		/* Add IRQ vector to NVIC */
		/* PA7 is connected to EXTI_Line7, which has EXTI9_5_IRQn vector */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
		/* Set priority */
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		/* Set sub priority */
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x02;
		/* Enable interrupt */
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		/* Add to NVIC */
		NVIC_Init(&NVIC_InitStruct);


	ultrasonAVG.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);



	  // Capteur US ARG :


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

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
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	ultrasonARG.init(GPIOA, GPIO_InitStruct, EXTI_InitStruct);




	/*
	 * Gestion des capteurs de fin de course :
	 */

	/* Définit la pin à utiliser pour chaque line */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0); // Porte droite ouverte
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1); // Porte gauche fermée
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); // Porte droite fermée
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource15); // Porte gauche ouverte

// Initialise la line 0

		EXTI_InitStruct.EXTI_Line = EXTI_Line0;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // déclenchement sur front montant
		EXTI_Init(&EXTI_InitStruct);

		/* Donne le channel correspondant */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
		/* Set priority */
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		/* Set sub priority */
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		/* Enable interrupt */
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		/* Add to NVIC */
		NVIC_Init(&NVIC_InitStruct);

// Initialise la line 1

		EXTI_InitStruct.EXTI_Line = EXTI_Line1;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // déclenchement sur front montant
		EXTI_Init(&EXTI_InitStruct);
		/* Donne le channel correspondant */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
		/* Set priority */
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		/* Set sub priority */
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		/* Enable interrupt */
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		/* Add to NVIC */
		NVIC_Init(&NVIC_InitStruct);


// Initialise la line 13

		EXTI_InitStruct.EXTI_Line = EXTI_Line13;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // déclenchement sur front montant
		EXTI_Init(&EXTI_InitStruct);
		/* Donne le channel correspondant */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
		/* Set priority */
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		/* Set sub priority */
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		/* Enable interrupt */
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		/* Add to NVIC */
		NVIC_Init(&NVIC_InitStruct);


// Initialise la line 15

		EXTI_InitStruct.EXTI_Line = EXTI_Line15;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // déclenchement sur front montant
		EXTI_Init(&EXTI_InitStruct);
		/* Donne le channel correspondant */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
		/* Set priority */
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
		/* Set sub priority */
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
		/* Enable interrupt */
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		/* Add to NVIC */
		NVIC_Init(&NVIC_InitStruct);

}


/*
 * Fonction de mise à jour des capteurs à ultrason
 */
void SensorMgr::refresh(MOVING_DIRECTION direction)
{
	currentTime = Millis();
	static uint8_t capteur = 0;

	if(currentTime - lastRefreshTime >= refreshDelay)
	{
		/*ultrasonARD.stopInterrupt();
		ultrasonAVD.stopInterrupt();
		ultrasonAVG.stopInterrupt();
		ultrasonARG.stopInterrupt();*/

		if (direction == NONE){
			if(capteur == 0)
			{
				ultrasonAVG.refresh();
			}
			if(capteur == 1)
			{
				ultrasonAVD.refresh();
			}
			if (capteur == 2)
			{
				ultrasonARG.refresh();
			}
			if (capteur == 3)
			{
				ultrasonARD.refresh();
			}
		}
		else if(direction == FORWARD){
			if((capteur == 0) | (capteur == 2))
				ultrasonAVG.refresh();

			if((capteur == 1) | (capteur == 3))
				ultrasonAVD.refresh();
		}
		else if(direction == BACKWARD){
			if((capteur == 0) | (capteur == 2))
				ultrasonARG.refresh();

			if((capteur == 1) | (capteur == 3))
				ultrasonARD.refresh();

		}
		capteur = (capteur+1)%4;  // On rafraichit les valeurs de chaque capteur un par un (un par appel de refresh)
		lastRefreshTime = currentTime;
	}
}


/*
 * Fonctions d'interruption des capteurs à ultrason
 */

void SensorMgr::sensorInterrupt(int pin){
/*
	static int lastpin = 0;
	static int count = 0;

	if((pin == lastpin && count > 2))
		serial.printfln("fail o %d", pin);
	else if((pin != lastpin && count < 2) && lastpin != 0)
		serial.printfln("fail u %d", lastpin);

	if(lastpin == pin)
			count += 1;
		else count = 1;
*/

	if(pin == 4)
		ultrasonARG.interruption();
	else if(pin == 6)
		ultrasonARD.interruption();
	else if(pin == 5)
		ultrasonAVD.interruption();
	else if(pin == 7)
		ultrasonAVG.interruption();

	//lastpin = pin;
}

void SensorMgr::AVDInterrupt(){
	ultrasonAVD.interruption();

}

void SensorMgr::AVGInterrupt(){
	ultrasonAVG.interruption();
}

void SensorMgr::ARDInterrupt(){
	ultrasonARD.interruption();
}

void SensorMgr::ARGInterrupt(){
	ultrasonARG.interruption();
}


/*
 * Fonctions de récupération de la distance mesurée
 */

int SensorMgr::getSensorDistanceAVG() {
	return ultrasonAVG.value();

}

/*
 * Fonctions de récupération de la distance mesurée
 */

int SensorMgr::getSensorDistanceAVD() {
	return ultrasonAVD.value();

}
/*
 * Fonctions de récupération de la distance mesurée
 */

int SensorMgr::getSensorDistanceARG() {
	return ultrasonARG.value();
}
/*
 * Fonctions de récupération de la distance mesurée
 */

int SensorMgr::getSensorDistanceARD() {
	return ultrasonARD.value();
}



/*
 * Fonctions de récupération de l'état des capteurs de contact et du jumper
 */


bool SensorMgr::isJumperOut() const{
	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);
}

bool SensorMgr::isLeftDoorOpen() {
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15);
}

bool SensorMgr::isRightDoorOpen() {
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0);
}

bool SensorMgr::isLeftDoorClosed() {
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1);
}

bool SensorMgr::isRightDoorClosed() {
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13);
}
