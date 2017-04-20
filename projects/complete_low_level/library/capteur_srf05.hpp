#ifndef CAPTEUR_SRF05_HPP
#define CAPTEUR_SRF05_HPP

#include <stdint.h>
#include <stm32f4xx_exti.h>
#include "delay.h"
#include "ring_buffer.hpp"
#include "Uart.hpp"

#define NB_VALEURS_MEDIANE_SRF  5

typedef ring_buffer<uint32_t, NB_VALEURS_MEDIANE_SRF> ringBufferSRF;
extern Uart<1> serial;

/** @file library/capteur_srf05.hpp
 *  @brief Ce fichier cr�e une classe capteur_srf05 pour pouvoir utiliser simplement les capteurs SRF05.
 *  @author Sylvain (adaptation du travail de Thibaut ~MissFrance~)
 *  @date 23 f�vrier 2015
 */

//Angle du c�ne de vision: 38�
//Distance maximale: 230cm


/** @class capteur_srf05
 *  \brief Classe pour pouvoir g�rer facilement les capteurs srf05.
 * 
 * 
 *  La classe g�re la r�cup�ration d'une distance entre le capteur et un obstacle.
 *  
 *  Protocole de ces capteurs :
 *  ---------------------------
 *
 *  La carte envoie une impulsion sur la pin pendant une dur�e de ~10�s. Puis, apr�s
 *  une dur�e inconnue, le capteur envoie une impulsion sur cette m�me pin. La dur�e
 *  de cette impulsion est proportionnelle � la distance entre les capteurs et l'objet
 *  d�tect�.
 */


class CapteurSRF
{
public:
	CapteurSRF()
	{
		derniereDistance = 0;
		origineTimer = 0;
		risingEdgeTrigger = true;
        dernierPing=Millis();

	}

	void init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef GPIO_sensor, EXTI_InitTypeDef EXTI_sensor)
	{
		this->GPIOx = GPIOx;
		this->GPIO_sensor = GPIO_sensor;
		this->EXTI_sensor = EXTI_sensor;
	}

	uint32_t value()
	{
		uint32_t valeurRetour = derniereDistance;
		derniereDistance = 0;
		return valeurRetour;
	}

	void refresh() {
        if (Millis() - dernierPing > 30) {
            EXTI_sensor.EXTI_LineCmd = DISABLE;
            EXTI_Init(&EXTI_sensor);
            // On met la pin en output
            GPIO_sensor.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_Init(GPIOx, &GPIO_sensor);

            // On met un z�ro sur la pin pour 2 �s
            GPIO_ResetBits(GPIOx, GPIO_sensor.GPIO_Pin);
            Delay_us(2);

            // On met un "un" sur la pin pour 10 �s
            GPIO_SetBits(GPIOx, GPIO_sensor.GPIO_Pin);
            Delay_us(10);
            GPIO_ResetBits(GPIOx, GPIO_sensor.GPIO_Pin);
            risingEdgeTrigger=true;
            dernierPing=Millis();

            // Le signal a �t� envoy�, maintenant on attend la r�ponse dans l'interruption
            GPIO_sensor.GPIO_Mode = GPIO_Mode_IN;
            GPIO_Init(GPIOx, &GPIO_sensor);
            EXTI_sensor.EXTI_Trigger = EXTI_Trigger_Rising;//On va maintenant recevoir un front montant, il faut se pr�parer pour �a
            EXTI_sensor.EXTI_LineCmd = ENABLE;                    //On accepte donc de lire les interruptions sur la pin du capteur � partir de maintenant
            EXTI_Init(&EXTI_sensor);

        }
    }

	/** Fonction appell�e par l'interruption. S'occupe d'enregistrer la valeur de la longueur
	 *  de l'impulsion retourn�e par le capteur, et de la convertir en une distance en mm.
	 */
	void interruption()
	{

		if(risingEdgeTrigger)
		{
			//serial.printflnDebug("risingEdgeTrigger");
			origineTimer = Micros();
			risingEdgeTrigger = false;
			EXTI_sensor.EXTI_Trigger = EXTI_Trigger_Falling;	//On devrait recevoir d�sormais un front descendant
			EXTI_Init(&EXTI_sensor);
		}
		else
		{
			uint32_t temps_impulsion, current_time;
			current_time = Micros();
			temps_impulsion = current_time - origineTimer;		//Le temps entre les deux fronts
			//derniereDistance = 10*temps_impulsion/58;
			ringBufferValeurs.append( 10*temps_impulsion/58 );	//On ajoute la distance mesur�e � cet instant dans un buffer, calcul� ainsi en fonction du temps entre les fronts
			derniereDistance = mediane(ringBufferValeurs);		//Ce qu'on renvoie est la m�diane du buffer, ainsi on �limine les valeurs extr�mes qui peuvent �tre absurde
/*
			serial.printflnDebug("%d", current_time);
			serial.printflnDebug("%d", origineTimer);
			serial.printflnDebug("%d", derniereDistance);
*/
			//risingEdgeTrigger = true;
			EXTI_sensor.EXTI_LineCmd = DISABLE;					//On a re�u la r�ponse qui nous int�ressait, on d�sactive donc les lectures d'interruptions sur ce capteur
			EXTI_Init(&EXTI_sensor);
			GPIO_sensor.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_Init(GPIOx, &GPIO_sensor);
		}
	}

	void stopInterrupt()
	{
		EXTI_sensor.EXTI_LineCmd = DISABLE;
		EXTI_Init(&EXTI_sensor);
	}

private:
	GPIO_InitTypeDef GPIO_sensor;//Variable permettant de r�gler les param�tres de la pin du capteur
	EXTI_InitTypeDef EXTI_sensor;//Variable permettant de r�gler le vecteur d'interruptions associ� au capteur
	GPIO_TypeDef* GPIOx;//Port de la pin du capteur
	ringBufferSRF ringBufferValeurs;
	volatile uint32_t derniereDistance;		//contient la derni�re distance acquise, pr�te � �tre envoy�e
	volatile uint32_t origineTimer;			//origine de temps afin de mesurer une dur�e
	bool risingEdgeTrigger;
    uint32_t dernierPing;
};

#endif
