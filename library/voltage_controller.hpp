
#include "stm32f4xx.h"
#include "safe_enum.hpp"
#include "utils.h"
#include "Uart.hpp"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_adc.h"
#include "MotionControlSystem.h"


/**
 * Permet de mesurer la tension de la LiPO et l'affiche sur un indicateur à 10 LEDs
 * @author discord & sa maman
 */
class Voltage_controller : public Singleton<Voltage_controller>
{
public:
	Voltage_controller()
	{
		counter = 0;
		blink = false;
		minimal_voltage = 2850;
		usb_voltage = 600;
		voltage_echelon = 80;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct); //Remplit avec les valeurs par défaut

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);


		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1; // BUZZER !!!
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOE, &GPIO_InitStruct);

		GPIO_StructInit(&GPIO_InitStruct);


		adc_configure();

	}

	void measure()
	{
		uint32_t ADCValue = 0;
		ADCValue = adc_convert();

		clear_leds();

		GPIO_ResetBits(GPIOE, GPIO_Pin_1);

		if(ADCValue >= voltage_echelon*9 + minimal_voltage )
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_6);
		}
		if(ADCValue >= voltage_echelon*8 + minimal_voltage )
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_4);
		}
		if(ADCValue >= voltage_echelon*7 + minimal_voltage )
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_2);
		}
		if(ADCValue >= voltage_echelon*6 + minimal_voltage )
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_0);
		}
		if(ADCValue >= voltage_echelon*5 + minimal_voltage )
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_11);
		}
		if(ADCValue >= voltage_echelon*4 + minimal_voltage )
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_15);
		}
		if(ADCValue >= voltage_echelon*3 + minimal_voltage )
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_10);
		}
		if(ADCValue >= voltage_echelon*2 + minimal_voltage )
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_8);
		}
		if(ADCValue >= voltage_echelon + minimal_voltage )
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_9);
		}
		if(ADCValue >= minimal_voltage )
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_7);
		}
		if(ADCValue < minimal_voltage)
		{

				if(ADCValue >= usb_voltage)
					GPIO_SetBits(GPIOE, GPIO_Pin_1);


				if(blink)
				{
					GPIO_SetBits(GPIOC, GPIO_Pin_7);
				}
				else
				{
					GPIO_ResetBits(GPIOC, GPIO_Pin_7);
				}
				blink = !blink;
			}
		}


	int test(){
		return(adc_convert());
	}
private:

	uint32_t voltage_echelon; //Cherches pas, y'a pas d'unité SI.
	uint32_t minimal_voltage; //Là non plus.
	uint32_t usb_voltage;
	int counter; //Là t'es con si t'en cherches une...
	bool blink;


	void adc_configure(){
	 ADC_InitTypeDef ADC_init_structure; //Structure for adc confguration
	 GPIO_InitTypeDef GPIO_initStructre; //Structure for analog input pin
	 //Clock configuration
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);//The ADC1 is connected the APB2 peripheral bus thus we will use its clock source
	 //RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
	 //Analog pin configuration
	 GPIO_initStructre.GPIO_Pin = GPIO_Pin_3;//	The channel 10 is connected to PC0
	 GPIO_initStructre.GPIO_Mode = GPIO_Mode_AN; //The PC0 pin is configured in analog mode
	 GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL; //We don't need any pull up or pull down
	 GPIO_Init(GPIOA,&GPIO_initStructre);//Affecting the port with the initialization structure configuration
	 //ADC structure configuration
	 ADC_DeInit();
	 ADC_init_structure.ADC_DataAlign = ADC_DataAlign_Right;//data converted will be shifted to right
	 ADC_init_structure.ADC_Resolution = ADC_Resolution_12b;//Input voltage is converted into a 12bit number giving a maximum value of 4096
	 ADC_init_structure.ADC_ContinuousConvMode = ENABLE; //the conversion is continuous, the input data is converted more than once
	 ADC_init_structure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;// conversion is synchronous with TIM1 and CC1 (actually I'm not sure about this one :/)
	 ADC_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//no trigger for conversion
	 ADC_init_structure.ADC_NbrOfConversion = 1;//I think this one is clear :p
	 ADC_init_structure.ADC_ScanConvMode = DISABLE;//The scan is configured in one channel
	 ADC_Init(ADC3,&ADC_init_structure);//Initialize ADC with the previous configuration
	 //Enable ADC conversion
	 ADC_Cmd(ADC3,ENABLE);
	 //Select the channel to be read from
	 ADC_RegularChannelConfig(ADC3,ADC_Channel_3,1,ADC_SampleTime_144Cycles);
	}

	void clear_leds()
	{
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
		GPIO_ResetBits(GPIOD, GPIO_Pin_4);
		GPIO_ResetBits(GPIOD, GPIO_Pin_2);
		GPIO_ResetBits(GPIOD, GPIO_Pin_0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_11);
		GPIO_ResetBits(GPIOA, GPIO_Pin_15);
		GPIO_ResetBits(GPIOA, GPIO_Pin_10);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		GPIO_ResetBits(GPIOC, GPIO_Pin_7);
	}


	int adc_convert(){
	 ADC_SoftwareStartConv(ADC3);//Start the conversion
	 while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC));//Processing the conversion
	 return ADC_GetConversionValue(ADC3); //Return the converted data
	}

};
