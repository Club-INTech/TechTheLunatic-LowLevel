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


// on définit les différents angles utilisés pour le coté gauche et le coté droit

#define fishingRightPositionUp 160
#define fishingRightPositionMid 155
#define fishingRightPositionDown 150
#define initialRightPosition 230
#define middleRightPosition 200
#define fingerRightInitialPosition 150
#define fingerRightFreePosition 50
#define passingRightPosition 165        // position du bras au dessus du filet pour libérer les poiscailles


// Coté gauche
#define fishingLeftPositionDown 150
#define fishingLeftPositionMid 145 //valeur peu êter un peu élevé
#define fishingLeftPositionUp 140
#define initialLeftPosition 70
#define middleLeftPosition 100
#define fingerLeftInitialPosition 50
#define fingerLeftFreePosition 160
#define passingLeftPosition 140

class ActuatorsMgr : public Singleton<ActuatorsMgr>
{
private:
	typedef Uart<2> serial_ax; // On utilise le port série 2 de la stm32
	AX<serial_ax>* ax12MagnetsRight; // Bras pour pecher les poissons
	AX<serial_ax>* ax12FingerRightFishes; // Doigt pour décrocher les poissons
	AX<serial_ax>* ax12MagnetsLeft; // Bras gauche pour pecher les poissons
	AX<serial_ax>* ax12FingerLeftFishes; // Doigt gauche pour décrocher les poissons


public:
	ActuatorsMgr()
	{
		ax12MagnetsRight = new AX<serial_ax>(0,0,1023); // (ID, Angle_min, Angle_Max)
		ax12FingerRightFishes = new AX<serial_ax>(1,0,1023);
		ax12MagnetsRight->init();
		ax12MagnetsLeft = new AX<serial_ax>(2,0,1023); // (ID, Angle_min, Angle_Max)
		ax12FingerLeftFishes = new AX<serial_ax>(3,0,1023);
		ax12MagnetsLeft->init();
	}

	~ActuatorsMgr()
	{
		delete(ax12MagnetsRight);
		delete(ax12FingerRightFishes);
		delete(ax12MagnetsLeft);
		delete(ax12FingerLeftFishes);
	}

	void setAllID(){ //Permet de regler les différents AX12
		int i;
		serial.printfln("Reglage des ID des AX12");
		serial.printfln("(brancher un AX12 a la fois)");
		serial.printf("\n");

		serial.printfln("Brancher ax12MagnetsRight");
		serial.read(i);
		ax12MagnetsRight->initIDB(0);
		serial.printfln("done");

		serial.printfln("Brancher ax12FreeRightFishes");
		serial.read(i);
		ax12FingerRightFishes->initIDB(1);
		serial.printfln("done");

		serial.printfln("Brancher ax12MagnetsLeft");
		serial.read(i);
		ax12MagnetsLeft->initIDB(2);
		serial.printfln("done");

		serial.printfln("Brancher ax12FreeLeftFishes");
		serial.read(i);
		ax12FingerLeftFishes->initIDB(3);
		serial.printfln("done");

		serial.printfln("Fin du reglage");
	}




	void fishingRightMid() {
		ax12MagnetsRight->changeSpeed(slowSpeed);
		ax12MagnetsRight->goTo(fishingRightPositionMid);
	}

	void fishingRightUp() {
			ax12MagnetsRight->changeSpeed(slowSpeed);
			ax12MagnetsRight->goTo(fishingRightPositionUp);
		}

	void fishingRightDown() {
			ax12MagnetsRight->changeSpeed(slowSpeed);
			ax12MagnetsRight->goTo(fishingRightPositionDown);
		}

	void fishingLeftMid() {
		ax12MagnetsLeft->changeSpeed(slowSpeed);
		ax12MagnetsLeft->goTo(fishingLeftPositionMid);
	}

	void fishingLeftUp() {
			ax12MagnetsLeft->changeSpeed(slowSpeed);
			ax12MagnetsLeft->goTo(fishingLeftPositionUp);
		}

	void fishingLeftDown() {
			ax12MagnetsLeft->changeSpeed(slowSpeed);
			ax12MagnetsLeft->goTo(fishingLeftPositionDown);
		}

	void midPositionRight() {
		ax12MagnetsRight->changeSpeed(slowSpeed);
		ax12MagnetsRight->goTo(middleRightPosition);
	}

	void midPositionLeft() {
		ax12MagnetsLeft->changeSpeed(slowSpeed);
		ax12MagnetsLeft->goTo(middleLeftPosition);
	}


	// FREE FKING FISHES


	void rightMagnetsDown() {
		ax12MagnetsRight->changeSpeed(slowSpeed);
		ax12MagnetsRight->goTo(passingRightPosition);
	}
	void rightFingerDown(){
		ax12FingerRightFishes->changeSpeed(fastSpeed);
		ax12FingerRightFishes->goTo(fingerRightFreePosition);
	}
	void rightMagnetsUp(){
		ax12MagnetsRight->changeSpeed(fastSpeed);
		ax12MagnetsRight->goTo(initialRightPosition);
	}
	void rightFingerUp(){
		ax12FingerRightFishes->changeSpeed(fastSpeed);
		ax12FingerRightFishes->goTo(fingerRightInitialPosition);
	}

	void leftMagnetsDown() {
		ax12MagnetsLeft->changeSpeed(slowSpeed);
		ax12MagnetsLeft->goTo(passingLeftPosition);
	}
	void leftFingerDown(){
		ax12FingerLeftFishes->changeSpeed(fastSpeed);
		ax12FingerLeftFishes->goTo(fingerLeftFreePosition);
	}
	void leftMagnetsUp(){
		ax12MagnetsLeft->changeSpeed(fastSpeed);
		ax12MagnetsLeft->goTo(initialLeftPosition);
	}
	void leftFingerUp(){
		ax12FingerLeftFishes->changeSpeed(fastSpeed);
		ax12FingerLeftFishes->goTo(fingerLeftInitialPosition);
	}


	// Voilà.


	void initialPositionFish() { //pour remettre AX12 dans leurs positions initiales

		ax12MagnetsLeft->changeSpeed(fastSpeed);
		ax12MagnetsLeft->goTo(initialLeftPosition);
		ax12MagnetsRight->changeSpeed(fastSpeed);
		ax12MagnetsRight->goTo(initialRightPosition);
		Delay(800);
		ax12FingerLeftFishes->changeSpeed(fastSpeed);
		ax12FingerLeftFishes->goTo(fingerLeftInitialPosition);
		ax12FingerRightFishes->changeSpeed(fastSpeed);
		ax12FingerRightFishes->goTo(fingerRightInitialPosition);

	}

	void setAXposMagnetsRight(int position) { // pour définir manuellement 1 position
		ax12MagnetsRight->goTo(position);
	}
	void setAXposMagnetsLeft(int position) { // pour définir manuellement 1 position
		ax12MagnetsLeft->goTo(position);
	}
	void setAXposFreeRightFishes(int position) { // pour définir manuellement 1 position
		ax12FingerRightFishes->goTo(position);
	}
	void setAXposFreeLeftFishes(int position) { // pour définir manuellement 1 position
		ax12FingerLeftFishes->goTo(position);
	}

	void changeAXSpeed(int speed) //fonction inutile faire changeSpeedB(speed)
	{
		ax12MagnetsLeft->changeSpeedB(speed);

	}
};

#endif /* ACTUATORSMGR_HPP */
