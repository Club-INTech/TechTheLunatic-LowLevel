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

#define slowSpeed 16
#define fastSpeed 25

#define brapeldepG 235
#define brapelrelG 135
#define brapeldepD 245
#define brapelrelD 150

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
	AX<serial_ax>* ax12test; // ax12 de test
    AX<serial_ax>* ax12brapelG;//ax12 pour le bras gauche de la pelle
    AX<serial_ax>* ax12brapelD;//ax12 pour le bras droit de la pelle

public:
	ActuatorsMgr()
	{
		ax12test = new AX<serial_ax>(0,0,1023); // (ID, Angle_min, Angle_Max)
		ax12test->init();
        ax12brapelG = new AX<serial_ax>(1,(uint16_t)1023*brapeldepG/300,(uint16_t)1023*brapelrelG/300); // (ID, Angle_min, Angle_Max)
        ax12brapelG->init();
        ax12brapelD = new AX<serial_ax>(2,(uint16_t)1023*brapelrelD/300,(uint16_t)1023*brapeldepD/300); // (ID, Angle_min, Angle_Max)
        ax12brapelD->init();
	}

	~ActuatorsMgr()
	{
		delete(ax12test);
        delete (ax12brapelG);
        delete (ax12brapelD);
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

        serial.printfln("Brancher ax12brapelG");
        serial.read(i);
        ax12brapelG->initIDB(1);
        serial.printfln("done");

        serial.printfln("Brancher ax12brapelD");
        serial.read(i);
        ax12brapelD->initIDB(2);
        serial.printfln("done");

    }

	void changeangle(uint16_t anglemin,uint16_t anglemax)
	{
		ax12test->changeAngleMIN((uint16_t )1023*anglemin/300);
		ax12test->changeAngleMAX((uint16_t)1023*anglemax/300);
	}



    void brapelreleve()
    {
        ax12brapelG->goTo(brapelrelG);
		ax12brapelD->goTo(brapelrelD);
    }
    void brapeldeplie()
    {
        ax12brapelG->goTo(brapeldepG);
		ax12brapelD->goTo(brapeldepD);
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

    /*bool change_actualpos(uint16_t position)
    {
        return (ax12test->change_actpos(position));
    }*/
    
    /*uint16_t posdeax12()
    {
        return getPositionDegres();
    }*/

    void reanimation () //réanime les ax12 en changeant leur baudrates
    {
        ax12test->reanimationMode(9600);
    }
};

#endif /* ACTUATORSMGR_HPP */
