/*
 * BinaryMotor.cpp
 *
 * Classe de gestion des moteurs sans PWM
 *
 */

#include "BinaryMotorMgr.hpp"
#include "SensorMgr.h"


bool rightDoorOpening = false;
bool leftDoorOpening = false;
bool rightDoorClosing = false;
bool leftDoorClosing = false;

bool RightDoorBlocked = false;
bool LeftDoorBlocked = false;


uint32_t timeToStopDoor = 800;

uint32_t timeTooLong = 6000;

BinaryMotorMgr::BinaryMotorMgr() {



		/**
		 * Configuration des pins :
		 * Porte Gauche ouverture : PE9
		 * Porte gauche fermeture : PE11
		 * Porte Droite ouverture : PE13
		 * Porte droite fermeture : PE15
		 *
		 * Enable porte Gauche : PB0
		 * Enable porte Droite : PB2
		 *
		 * Axe gauche : PE5
		 * Axe droit : PE3
		 *
		 */

		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut
		// Active l'horloge du port D
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);


			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOE, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_ResetBits(GPIOE, GPIO_Pin_9);
			GPIO_ResetBits(GPIOE, GPIO_Pin_11);
			GPIO_ResetBits(GPIOE, GPIO_Pin_13);
			GPIO_ResetBits(GPIOE, GPIO_Pin_15);

			GPIO_SetBits(GPIOB, GPIO_Pin_0);
			GPIO_SetBits(GPIOB, GPIO_Pin_2);

}

/*--- Axes rotatifs ---*/

void BinaryMotorMgr::runAxisLeft() {
	GPIO_SetBits(GPIOE, GPIO_Pin_5);
	}

void BinaryMotorMgr::runAxisRight() {
	GPIO_SetBits(GPIOE, GPIO_Pin_3);
	}

void BinaryMotorMgr::stopAxisLeft() {
	GPIO_ResetBits(GPIOE, GPIO_Pin_5);
	}

void BinaryMotorMgr::stopAxisRight() {
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	}


/*--- Mouvement des portes ---*/

void BinaryMotorMgr::runForwardLeft() {

	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);

}

void BinaryMotorMgr::runBackwardLeft() {

	GPIO_SetBits(GPIOE, GPIO_Pin_11);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
}

void BinaryMotorMgr::runForwardRight() {

	GPIO_ResetBits(GPIOE, GPIO_Pin_15);
	GPIO_SetBits(GPIOE, GPIO_Pin_13);
}

void BinaryMotorMgr::runBackwardRight() {

	GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	GPIO_SetBits(GPIOE, GPIO_Pin_15);
}

void BinaryMotorMgr::stopLeftDoor() {
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);


}

void BinaryMotorMgr::stopRightDoor() {
	GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	GPIO_ResetBits(GPIOE, GPIO_Pin_15);

}



