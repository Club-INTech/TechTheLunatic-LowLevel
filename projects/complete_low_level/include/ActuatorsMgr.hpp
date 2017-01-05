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

// vitesse des AX12

#define slowSpeed 16
#define fastSpeed 25

//Positions des AX12

//Pour la Pelleteusatron 3000

#define brapeldepG 30
#define brapelmoitG 60
#define brapelrelG 90

#define pospelinit 300
#define pospelmoit 150
#define pospeldeli 0

//Pour l'attrape-module

//TODO:positions des deux AX12 d'attrappe-module

#define AMdebG 0
#define AMmidG 0
#define AMfinG 0
#define AMdebD 0
#define AMmidD 0
#define AMfinD 0

class ActuatorsMgr : public Singleton<ActuatorsMgr>
{
private:
	typedef Uart<2> serial_ax;  // On utilise le port série 2 de la stm32
	AX<serial_ax>* ax12test;    // ax12 de test
	AX<serial_ax>* ax12brapel;  //objet gérant les deux AX12 des bras
	//AX<serial_ax>* ax12brapelD;
	AX<serial_ax>* ax12pel;     //ax12 pour la pelle de la pelleteuse
	
	//AX12 de l'attrape module gauche et droit:
	AX<serial_ax>* AMG;
	AX<serial_ax>* AMD;

public:
	ActuatorsMgr()
	{
		ax12test = new AX<serial_ax>(0,0,1023); // (ID, Angle_min, Angle_Max)
		ax12test->init();
		ax12brapel = new AX<serial_ax>(1,(uint16_t)1023*brapeldepG/300,(uint16_t)1023*brapelrelG/300); // (ID, Angle_min, Angle_Max)
		ax12brapel->init();
		//  ax12brapelD = new AX<serial_ax>(1,(uint16_t)1023*brapeldepD/300,(uint16_t)1023*brapelrelD/300); // (ID, Angle_min, Angle_Max)
		//  ax12brapelD->init();
		ax12pel = new AX<serial_ax>(2,0,1023);
		ax12pel->init();
		AMG = new AX<serial_ax>(3,0,1023);
		AMG->init();
		AMD = new AX<serial_ax>(4,0,1023);
		AMD->init();
		
	}
	
	~ActuatorsMgr()
	{
		delete(ax12test);
		delete(ax12brapel);
		// delete(ax12brapelD);
		delete(ax12pel);
		delete(AMD);
		delete(AMG);
	}
	
	void setAllID(){ //Permet de regler les IDs des différents AX12
		int i;
		serial.printfln("Reglage des ID des AX12");
		serial.printfln("(brancher un AX12 a la fois)");
		serial.printf("\n");
		
		serial.printfln("Brancher ax12test");
		serial.read(i);
		ax12test->initIDB(0);
		ax12test->init();
		serial.printfln("done");
		
		serial.printfln("Brancher ax12brapelG et ax12brapelD");
		serial.read(i);
		ax12brapel->initIDB(1);
		ax12brapel->init();
		//  ax12brapelD->initIDB(1);
		serial.printfln("done");
/*
        serial.printfln("Brancher ax12brapelD");
        serial.read(i);
        ax12brapelD->initIDB(1);
        serial.printfln("done");
*/
		serial.printfln("Brancher ax12pel");
		serial.read(i);
		ax12pel->initIDB(2);
		ax12pel->init();
		serial.printfln("done");
		
	}
	
	void changeangle(uint16_t anglemin,uint16_t anglemax) //permet de modifier les angles max et min de l'ax12 de test
	{
		ax12test->changeAngleMIN((uint16_t )1023*anglemin/300);
		ax12test->changeAngleMAX((uint16_t)1023*anglemax/300);
	}


/*			 ____________________
 * 		   *|                    |*
 *		   *|    Pelle T-3000    |*
 *		   *|____________________|*
 */
	
	
	void braPelReleve() //relève les bras de la pelle
	{
		serial.printfln("Leve les bras");
		// ax12brapelD->changeSpeed(25);
		ax12brapel->changeSpeed(25);
		ax12brapel->goTo(brapelrelG);
		//	ax12brapelD->goTo(brapelrelD);
		serial.printfln("done");
	}
	void braPelDeplie() // déplie les bras de la pelle
	{
		serial.printfln("Baisse les bras");
		//  ax12brapelD->changeSpeed(25);
		ax12brapel->changeSpeed(16);
		ax12brapel->goTo(brapeldepG);
		//	ax12brapelD->goTo(brapeldepD);
		serial.printfln("done");
	}
	void braPelMoit()
	
	{
		serial.printfln("Leve les bras mais pas trop");
		// ax12brapelD->changeSpeed(15);
		ax12brapel->changeSpeed(20);
		ax12brapel->goTo(brapelmoitG);
		//ax12brapelD->goTo(brapelmoitD);
		serial.printfln("done");
	}
	void pelleInit()
	{
		serial.printfln("Pelle va au début");
		ax12pel->changeSpeed(40);
		ax12pel->goTo(pospelinit);
		serial.printfln("done");
	}
	void pelleMoit()
	{
		serial.printfln("Pelle tient boules");
		ax12pel->changeSpeed(25);
		ax12pel->goTo(pospelmoit);
		serial.printfln("done");
	}
	void pelleLib()
	{
		serial.printfln("Pelle jete boules");
		ax12pel->changeSpeed(35);
		ax12pel->goTo(pospeldeli);
		serial.printfln("done");
	}

/*			 ___________________
 * 		   *|                   |*
 *		   *|  Attrappe Module  |*
 *		   *|___________________|*
 */
	
	void moduleDeb(int cote)
	{
		serial.printfln("Initialisation de l'attrape module");
		if (cote)
		{
			AMG->goTo(AMdebG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMdebD);  //Si le côté est droit (cote = 0)
		}
		serial.printfln("done");
	}
	void moduleMoit(int cote)
	{
		serial.printfln("Position d'évitement de la calle de l'attrapeM");
		if (cote)
		{
			AMG->goTo(AMmidG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMmidD); //Si le côté est droit (cote = 0)
		}
		serial.printfln("done");
	}
	void moduleFin(int cote)
	{
		serial.printfln("Prise de modules");
		if (cote)
		{
			AMG->goTo(AMfinG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMfinD);  //Si le côté est droit (cote = 0)
		}
		serial.printfln("done");
	}
	// Voilà.
	
	
	
	void setAXpos(uint16_t  position) { // pour d�finir manuellement 1 position
		ax12test->goTo(position);
		
	}
	
	void changeAXSpeed(int speed) //fonction inutile faire changeSpeedB(speed)
	{
		ax12test->changeSpeedB(speed);
		
	}
	
	bool change_actualpos(uint16_t position)//ne marche pas (permettrait de changer la position de l'ax12 sans le faire bouger)
	{
		return (ax12test->change_actpos(position));
	}
	
	uint16_t posdeax12()//ne marche pas( permettrait d'obtenir la position des ax12)
	{
		return ax12test->getPositionDegres();
	}
	
	void reanimation () //réanime les ax12 en changeant leur baudrates
	{
		ax12test->reanimationMode(9600);
	}
};

#endif /* ACTUATORSMGR_HPP */
