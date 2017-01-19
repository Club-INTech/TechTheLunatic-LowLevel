/**
 * Gestion de l'ascenseur
 *
 * Pins du moteur:
 * Sens: PE9|PE11 en OUT
 * Marche: PB0 en PWM (TIM3_CH3)
 *
 *
 * Pins de sens du moteur (bornier gauche):
 * PE9(INPUT1) PE11(INPUT2) en mode OUTPUT
 *
 * SENS: en vue du dessus: PE9/PE11
 * Trigonométrique:1/0
 * Antitrigonomètrique:0/1
 *
 */


#define GREEN_PIN       GPIO_Pin_12
#define ORANGE_PIN      GPIO_Pin_13
#define RED_PIN         GPIO_Pin_14
#define BLUE_PIN        GPIO_Pin_15

#include <delay.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx.h>
#include "Elevator.h"

Elevator::Elevator(void) {
	sens = UP;
}

void Elevator::initTimer()  //Initialise le timer
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseInitTypeDef timTimeBaseInitTypeDef;
	TIM_TimeBaseStructInit(&timTimeBaseInitTypeDef);
	
	uint16_t prescaler = (uint16_t)((SystemCoreClock / 2) / 256000) - 1; //CF Motor.cpp
	
	timTimeBaseInitTypeDef.TIM_Period=1000;
	timTimeBaseInitTypeDef.TIM_ClockDivision=TIM_CKD_DIV1;
	timTimeBaseInitTypeDef.TIM_Prescaler=prescaler;
	timTimeBaseInitTypeDef.TIM_CounterMode=TIM_CounterMode_Up;
	//Configuration du TIMER 3
	
	TIM_TimeBaseInit(TIM3, &timTimeBaseInitTypeDef);
}

void Elevator::initPWM() //Initialise le PWM
{
	TIM_OCInitTypeDef timOcInitTypeDef;
	
	timOcInitTypeDef.TIM_OCMode=TIM_OCMode_PWM1;
	timOcInitTypeDef.TIM_OutputState=TIM_OutputState_Enable;
	timOcInitTypeDef.TIM_OCPolarity=TIM_OCPolarity_High;
	timOcInitTypeDef.TIM_Pulse=0; //PWM initial nul
	
	TIM_OC3Init(TIM3, &timOcInitTypeDef); //Canal 2N de TIM1
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	
	TIM_Cmd(TIM3, ENABLE); //Active le TIM
}

void Elevator::initPins(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef gpioPinInitStruct;
	GPIO_StructInit(&gpioPinInitStruct);
	
	gpioPinInitStruct.GPIO_Pin=GPIO_Pin_9 + GPIO_Pin_11;  //Pins de sens
	gpioPinInitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_Init(GPIOE, &gpioPinInitStruct);
	
	gpioPinInitStruct.GPIO_Pin=GPIO_Pin_0;                  //Pin de pwm
	gpioPinInitStruct.GPIO_Mode=GPIO_Mode_AF;
	
	gpioPinInitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	gpioPinInitStruct.GPIO_OType=GPIO_OType_PP;
	gpioPinInitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &gpioPinInitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3);
	
	//Initialise le moteur dans le sens montant
	
	GPIO_SetBits(GPIOE, GPIO_Pin_11 + GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);
}

void Elevator::setSens(Sens sensToSet) { //Change la direction dans le sens souhaité(UP ou DOWN)
	if (sensToSet==UP){
		sens=UP;
		GPIO_SetBits(GPIOE, GPIO_Pin_11);
		GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	}
	else if (sensToSet==DOWN){
		sens=DOWN;
		GPIO_SetBits(GPIOE, GPIO_Pin_9);
		GPIO_ResetBits(GPIOE, GPIO_Pin_11);
	}
}

void Elevator::stop(void){
	TIM3->CCR3=0;
	if(sens==UP){
		sens=DOWN;
	}
	else if(sens==DOWN){
		sens=UP;
	}
}

void Elevator::run() {//Tourne dans le sens de sens(a déterminer empiriquement)
	TIM3->CCR3=200;
}

void Elevator::initialize(void){
	initPins();
	initTimer();
	initPWM();
}