/**
 * Gestion de l'ascenseur
 *
 * Pins du moteur:
 * Sens: PE9|PE11 en OUT
 * Marche: PB0 en PWM (TIM1_CH2)
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

//TODO:Pourquoi l'un des moteurs avait son ENABLE sur le PB2 (qui n'a pas de timer à priori)


#define GREEN_PIN       GPIO_Pin_12
#define ORANGE_PIN      GPIO_Pin_13
#define RED_PIN         GPIO_Pin_14
#define BLUE_PIN        GPIO_Pin_15

#include <delay.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx.h>
#include "Elevator.h"

Elevator::Elevator(void) {
	sens = UP;

	 

	/*
   GPIO_InitTypeDef gpioInitTypeDef;
   GPIO_StructInit(&gpioInitTypeDef); //Initialise les valeurs par défaut
   
   //Activation de l'horloge du port E:
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
   
   //Initialise les pins de sens dans le sens initial(UP)
   gpioInitTypeDef.GPIO_Pin=GPIO_Pin_13 | GPIO_Pin_15;
   gpioInitTypeDef.GPIO_Mode=GPIO_Mode_OUT;
   gpioInitTypeDef.GPIO_Speed=GPIO_Speed_100MHz;
   GPIO_Init(GPIOE, &gpioInitTypeDef);

*/
}

void Elevator::initTimer()  //Initialise le timer
{
	
	/*
	uint32_t TIMER_Frequency = 72000000;
	uint32_t COUNTER_Frequency = 100000;
	uint32_t PSC_Value = (TIMER_Frequency / COUNTER_Frequency) - 1;
	uint16_t ARR_Value = 600;
	
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_TimeBaseStructInit (&TIM_TimeBaseStructure);

	TIM_TimeBaseStructure.TIM_Period = ARR_Value;
	TIM_TimeBaseStructure.TIM_Prescaler = PSC_Value;
	TIM_TimeBaseInit (TIM4, &TIM_TimeBaseStructure);
*/
}

void Elevator::initPWM() //Initialise le PWM
{
	
	/*
	//Leds:
	
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	// always initialise local variables before use
	TIM_OCStructInit (&TIM_OCInitStructure);
	
	// Common settings for all channels
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	//Green LED
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	//Channel2 - ORANGE LED*/
	
	/* Channel3 - RED LED
	TIM_OC3Init (TIM4, &TIM_OCInitStructure);
	
	// Channel4 - BLUE LED
	// make this the opposite polarity to the other two
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC4Init (TIM4, &TIM_OCInitStructure);
	TIM_OC2Init (TIM4, &TIM_OCInitStructure);
	TIM_Cmd(TIM4, ENABLE);
	
	//PWM:
	
	TIM_OCInitTypeDef timOcInitTypeDef;
	TIM_OCStructInit(&timOcInitTypeDef);
	timOcInitTypeDef.TIM_OCMode=TIM_OCMode_PWM1;
	timOcInitTypeDef.TIM_OutputState=TIM_OutputState_Enable;
	timOcInitTypeDef.TIM_OCPolarity=TIM_OCPolarity_High;
	timOcInitTypeDef.TIM_Pulse=0;
	TIM_OC2Init(TIM1, &timOcInitTypeDef); //canal 2 de tim1
	TIM_Cmd(TIM1, ENABLE);
	*/
}

void Elevator::pinsInit(void)
{
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef gpioPinsDir;
	GPIO_StructInit(&gpioPinsDir);
	GPIO_InitTypeDef gpioPinEn;
	GPIO_StructInit(&gpioPinEn);
	
	gpioPinsDir.GPIO_Pin=GPIO_Pin_9 + GPIO_Pin_11;
	gpioPinEn.GPIO_Pin=GPIO_Pin_0;
	
	gpioPinsDir.GPIO_Mode=GPIO_Mode_OUT;
	gpioPinEn.GPIO_Mode=GPIO_Mode_OUT;
	
	GPIO_Init(GPIOE, &gpioPinsDir);
	GPIO_Init(GPIOB, &gpioPinEn);
	
	GPIO_SetBits(GPIOE, GPIO_Pin_11 + GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);
	
	/*
	GPIO_InitTypeDef  GPIO_InitStructure;
	// always initialise local variables before use
	GPIO_StructInit (&GPIO_InitStructure);
	
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD, ENABLE);
	
	// these pins will be controlled by the CCRx registers
	GPIO_InitStructure.GPIO_Pin = ORANGE_PIN + RED_PIN + BLUE_PIN + GREEN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init (GPIOD, &GPIO_InitStructure);
	
	// ensure that the pins all start off in a known state
	GPIO_ResetBits (GPIOD, ORANGE_PIN + RED_PIN + BLUE_PIN + GREEN_PIN);
	
	// this one is used with delay_ms() to act as a timing reference
	
	// The others get connected to the AF function for the timer
	GPIO_PinAFConfig (GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
	GPIO_PinAFConfig (GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	GPIO_PinAFConfig (GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
	GPIO_PinAFConfig (GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);
	
	 */
}


void Elevator::switchSens() { //Inverse le sens
	if (sens==UP){
		sens=DOWN;
	}
	else if (sens==DOWN){
		sens=UP;
	}
	GPIO_ToggleBits(GPIOE, GPIO_Pin_11 + GPIO_Pin_9);
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

void Elevator::stop(void){ //
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	switchSens();
}

void Elevator::run() {//Tourne dans le sens de sens(a déterminer empiriquement)
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	
	/*
	 * Delay_Init();
	int intensite = 0;
	int increment = 1;
	int sens = 0;
	while (1) {
		TIM4->CCR1 = intensite;
		TIM4->CCR2 = intensite;
		TIM4->CCR3 = intensite;
		TIM4->CCR4 = intensite;
		TIM1->CCR2 = intensite;
		
		if (intensite > 599)
			sens = 1;
		if (intensite < 1)
			sens = 0;
		
		if (sens == 0)
			intensite += increment;
		else
			intensite -= increment;
		Delay_us(5000);
		
	}
	 */
	
	/*
	while(1){
		for (int i=0; i<30;i++){
			Delay_us(20000);
			intensite+=increment;
			TIM_SetCompare3(TIM4, intensite);
			intensite+=increment;
			TIM_SetCompare2(TIM4, intensite);
			intensite+=increment;
			TIM_SetCompare4(TIM4,intensite);
			intensite+=increment;
			TIM_SetCompare1(TIM4, intensite);
			
		}
		increment=-increment;
	}
	 */
	
}
