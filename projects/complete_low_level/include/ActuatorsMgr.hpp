#ifndef ACTUATORSMGR_HPP
#define ACTUATORSMGR_HPP

#include <ax12.hpp>
#include <Uart.hpp>
#include <Singleton.hpp>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"

extern Uart<1> serial;

#define position1 100
#define position2 200

// vitesse des AX12

#define slowSpeed 14
#define fastSpeed 25


// on d�finit les diff�rents angles utilis�s pour le cot� gauche et le cot� droit

#define fishingRightPositionUp 160
#define fishingRightPositionMid 155
#define fishingRightPositionDown 150
#define initialRightPosition 230
#define middleRightPosition 200
#define fingerRightInitialPosition 150
#define fingerRightFreePosition 50
#define passingRightPosition 165        // position du bras au dessus du filet pour lib�rer les poiscailles


// Cot� gauche
#define fishingLeftPositionDown 150
#define fishingLeftPositionMid 145 //valeur peu �ter un peu �lev�
#define fishingLeftPositionUp 140
#define initialLeftPosition 70
#define middleLeftPosition 100
#define fingerLeftInitialPosition 50
#define fingerLeftFreePosition 160
#define passingLeftPosition 140

class ActuatorsMgr : public Singleton<ActuatorsMgr>
{
private:
	typedef Uart<2> serial_ax; // On utilise le port s�rie 2 de la stm32
	AX<serial_ax>* ax12test; // Bras pour pecher les poissons


public:
	ActuatorsMgr()
	{
		ax12test = new AX<serial_ax>(0,0,1023); // (ID, Angle_min, Angle_Max)
		ax12test->init();
	}

	~ActuatorsMgr()
	{
		delete(ax12test);
	}

	void setAllID(){ //Permet de regler les diff�rents AX12
		int i;
		serial.printfln("Reglage des ID des AX12");
		serial.printfln("(brancher un AX12 a la fois)");
		serial.printf("\n");

		serial.printfln("Brancher ax12test");
		serial.read(i);
		ax12test->initIDB(0);
		serial.printfln("done");

	}




	void fishingRightMid() {
		ax12test->changeSpeed(slowSpeed);
		ax12test->goTo(fishingRightPositionMid);
	}






	// FREE FKING FISHES


	void rightMagnetsDown() {
		ax12test->changeSpeed(slowSpeed);
		ax12test->goTo(passingRightPosition);
	}
	void rightMagnetsUp(){
		ax12test->changeSpeed(fastSpeed);
		ax12test->goTo(initialRightPosition);
	}




	// Voil�.


	void initialPositionFish() { //pour remettre AX12 dans leurs positions initiales


		ax12test->changeSpeed(fastSpeed);
		ax12test->goTo(initialRightPosition);
		Delay(800);

	}

	void setAXpos(uint16_t  position) { // pour d�finir manuellement 1 position
		ax12test->goTo(position);
	}

	void changeAXSpeed(int speed) //fonction inutile faire changeSpeedB(speed)
	{
		ax12test->changeSpeedB(speed);

	}
};

#endif /* ACTUATORSMGR_HPP */
