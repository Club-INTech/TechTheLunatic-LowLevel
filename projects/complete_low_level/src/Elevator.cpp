/**
 * Gestion de l'ascenseur
 *
 * Pins du moteur:
 * Sens: PE15|PE13
 * Marche: PB2
 */

#include "Elevator.h"

#define UP 1
#define DOWN 0

//TODO:voir fonctionnement de initPWM dans motor.cpp


Elevator::Elevator(void){
	dir=UP;
	/**
	 * Pins de sens du moteur (bornier gauche):
	 * PE15(INPUT4) PE13(INPUT3)
	 */
	
	GPIO_InitTypeDef gpioInitTypeDef;
	GPIO_StructInit(&gpioInitTypeDef); //Initialise les valeurs par défaut
	
	//Activation de l'horloge du port E:
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//Initialise les pins de sens dans le sens initial(UP)
	gpioInitTypeDef.GPIO_Pin=GPIO_Pin_13 | GPIO_Pin_15;
	gpioInitTypeDef.GPIO_Mode=GPIO_Mode_OUT;
	gpioInitTypeDef.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &gpioInitTypeDef);
	
	
}

void Elevator::initTimer()  //Initialise le timer
{
	GPIO_InitTypeDef gpioInitTypeDef;
}
void Elevator::initPWM() //Initialise le PWM
{
	
}

void Elevator::initLED(){ //Initialise les leds
	
}

void Elevator::switchDir() { //Inverse le sens
	if (dir==UP){
		dir=DOWN;
	}
	else if (dir=DOWN){
		dir=UP;
	}
}

void Elevator::setDirection(int dirToSet) { //Change la direction dans le sens souhaité(UP ou DOWN)
	dir=dirToSet;
}

void Elevator::run() { //Tourne dans le sens de dir(a déterminer empiriquement)
	
}