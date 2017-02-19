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

//Vitesses AX12
//Pelle
#define vPelleDeb 25
#define vPelleMid 20
#define vPelleFin 45

//Bras Pelle
#define vBraPelRel 20
#define vBraPelMed 15
#define vBraPelDep 10

#define vCale 50


//Positions des AX12

//Pour la Pelleteusatron 3000

#define brapeldep 130
#define brapelmoit 160
#define brapelrel 180
#define pospelinit 300
#define pospeltient 105
#define pospelmoit 150
#define pospeldeli 0

//Pour les attrape-modules

#define AMdebG 240
#define AMmidG 150
#define AMfinG 50 // 60
#define AMdebD 70
#define AMmidD 150
#define AMfinD 245

//Les cale-modules
#define CaleHautG 120 //gauche 96 droite 183
#define CaleReposG 200
#define CaleBasG 215 //gauche 280 droite 3
#define CaleHautD 110
#define CaleReposD 45
#define CaleBasD 10

//Largue modules
#define LargueRepos 83
#define LarguePousse 44

class ActuatorsMgr : public Singleton<ActuatorsMgr>
{
private:
	typedef Uart<2> serial_ax;  // On utilise le port série 2 de la stm32
	AX<serial_ax>* ax12test;    // ax12 de test
	AX<serial_ax>* ax12brapel;  //objet gérant les deux AX12 des bras
	AX<serial_ax>* ax12pel;     //ax12 pour la pelle de la pelleteuse
	
	//AX12 de l'attrape module gauche et droit:
	AX<serial_ax>* AMG;
	AX<serial_ax>* AMD;
	
	//AX12 pour les calle modules:
	AX<serial_ax>* CMD;//droit
	AX<serial_ax>* CMG;//gauche
	
	//AX12 pour le largueur
	AX<serial_ax>* LM;

public:
	ActuatorsMgr()
	{
		ax12test = new AX<serial_ax>(0,0,1023); // (ID, Angle_min, Angle_Max)
		ax12test->init();
		ax12brapel = new AX<serial_ax>(1,0,1023); // (ID, Angle_min, Angle_Max)
		ax12brapel->init();
		ax12pel = new AX<serial_ax>(2,0,1023);
		ax12pel->init();
		AMG = new AX<serial_ax>(3,0,1023);
		AMG->init();
		AMD = new AX<serial_ax>(4,0,1023);
		AMD->init();
		CMD = new AX<serial_ax>(5,0,1023);
		CMD->init();
		CMG = new AX<serial_ax>(6,0,1023);
		CMG->init();
		LM = new AX<serial_ax>(7,0,1023);
		LM->init();
	}
	
	~ActuatorsMgr()
	{
		delete(ax12test);
		delete(ax12brapel);
		delete(ax12pel);
		delete(AMD);
		delete(AMG);
		delete(CMD);
		delete(CMG);
		delete(LM);
	}
	
	void setAllID(){ //Permet de regler l'id d'un ax12
		int i;
		serial.printfln("Reglage de l'ID d'un AX12");
		serial.printf("\n");
		serial.printfln("Brancher l'AX12 à régler");
		serial.read(i);
		ax12test->initIDB(0);
		ax12test->init();
		serial.printfln("done");
	}
	
	void setPelleID(){ //Pour regler l'id des ax12 de la pelle
		int i;
		
		serial.printfln("Reglage de l'ID des AX12 de la pelle");
		serial.printfln("Brancher brancher les deux AX12 des bras de la pelleteuse");
		serial.read(i);
		ax12brapel->initIDB(1);
		ax12brapel->init();
		serial.printfln("done");
		
		serial.printfln("Brancher l'AX12 de la pelle");
		serial.read(i);
		ax12pel->initIDB(2);
		ax12pel->init();
		serial.printfln("done");
	}
	
	void setModuleID(){ //Pour regler l'id des ax12 de l'attrappe module
		int i;
		
		serial.printfln("Reglage de l'ID des AX12 de l'attrappe module");
		serial.printfln("Brancher l'AX12 droit de l'AM");
		serial.read(i);
		AMD->initIDB(4);
		AMD->init();
		serial.printfln("done");
				
		serial.printfln("Brancher l'AX12 gauche de l'AM");
		serial.read(i);
		AMG->initIDB(3);
		AMG->init();
		serial.printfln("done");
				
		serial.printfln("Brancher les AX12 droit des calleurs");
		serial.read(i);
		CMD->initIDB(5);
		CMD->init();
		serial.printfln("done");
		
		serial.printfln("Brancher les AX12 gauche des calleurs");
		serial.read(i);
		CMG->initIDB(6);
		CMG->init();
		serial.printfln("done");
		
		serial.printfln("Brancher l'AX12 du largueur");
		serial.read(i);
		LM->initIDB(7);
		LM->init();
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
		serial.printflnDebug("Leve les bras");
		ax12brapel->changeSpeed(20);
		ax12brapel->goTo(brapelrel);
		serial.printflnDebug("done");
	}
	
	void braPelDeplie() // déplie les bras de la pelle
	{
		serial.printflnDebug("Baisse les bras");
		ax12brapel->changeSpeed(25);
		ax12brapel->goTo(brapeldep);
		serial.printflnDebug("done");
	}
	
	void braPelMoit()
	{
		serial.printflnDebug("Leve les bras mais pas trop");
		ax12brapel->changeSpeed(10);
		ax12brapel->goTo(brapelmoit);
		serial.printflnDebug("done");
	}
	
	void pelleInit()
	{
		serial.printflnDebug("Pelle va au début");
		ax12pel->changeSpeed(50);
		ax12pel->goTo(pospelinit);
		serial.printflnDebug("done");
	}
	
	void pelleMoit()
	{
		serial.printflnDebug("Pelle tient boules");
		ax12pel->changeSpeed(20);
		ax12pel->goTo(pospelmoit);
		serial.printflnDebug("done");
	}
	
	void pelleTient(){
		serial.printflnDebug("pos de maintient des boules");
		ax12pel->changeSpeed(30);
		ax12pel->goTo(pospeltient);
		serial.printflnDebug("done");
	}
	
	void pelleLib()
	{
		serial.printflnDebug("Pelle jete boules");
		ax12pel->changeSpeed(50);
		ax12pel->goTo(pospeldeli);
		serial.printflnDebug("done");
	}

/*			 ___________________
 * 		   *|                   |*
 *		   *|  Attrappe Module  |*
 *		   *|___________________|*
 */
	
	void moduleDeb(int cote)
	{
		AMG->changeSpeed(100);
		AMD->changeSpeed(100);
		serial.printflnDebug("Initialisation de l'attrape module");
		if (cote)
		{
			AMG->goTo(AMdebG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMdebD);  //Si le côté est droit (cote = 0)
		}
		serial.printflnDebug("done");
	}
	
	void moduleMid(int cote)
	{
		AMG->changeSpeed(70);
		AMD->changeSpeed(70);
		serial.printflnDebug("Medium de l'attrape module");
		if (cote)
		{
			AMG->goTo(AMmidG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMmidD);  //Si le côté est droit (cote = 0)
		}
		serial.printflnDebug("done");
	}
	
	void moduleFin(int cote)
	{
		AMG->changeSpeed(70);
		AMD->changeSpeed(70);
		serial.printflnDebug("Prise de modules");
		if (cote)
		{
			AMG->goTo(AMfinG); //Si le coté est gauche (cote = 1)
		}
		else
		{
			AMD->goTo(AMfinD);  //Si le côté est droit (cote = 0)
		}
		serial.printflnDebug("done");
	}

/*			 ___________________
 * 		   *|                   |*
 *		   *|    Cale Module    |*
 *		   *|___________________|*
 */

	void caleHautD(){
		CMD->changeSpeed(vCale);
		CMD->goTo(CaleHautD);
	}
	void caleMidD(){
		CMD->changeSpeed(vCale);
		CMD->goTo(CaleReposD);
	}
	void caleBasD(){
		CMD->changeSpeed(vCale);
		CMD->goTo(CaleBasD);
	}
	
	void caleHautG(){
		CMG->changeSpeed(vCale);
		CMG->goTo(CaleHautG);
	}
	void caleMidG(){
		CMG->changeSpeed(vCale);
		CMG->goTo(CaleReposG);
	}
	void caleBasG(){
		CMG->changeSpeed(vCale);
		CMG->goTo(CaleBasG);
	}
/*			 ___________________
 * 		   *|                   |*
 *		   *|   Largue-Module   |*
 *		   *|___________________|*
 */
	void largueRepos(){
		LM->changeSpeed(50);
		LM->goTo(LargueRepos);
	}
	void larguePousse(){
		LM->changeSpeed(30);
		LM->goTo(LarguePousse);
	}
	void lmReasserv(){
		LM->unasserv();
		LM->asserv();
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
	
	void pelreasserv(){
		ax12brapel->init();
	}
	void setPunch(){
		uint16_t punchL=32;
		uint16_t punchH=0;
		serial.printflnDebug("Entrez punch_H(défaut=0)");
		serial.read(punchH);
		serial.printflnDebug("Entrez punch_L(défaut=32)");
		serial.read(punchL);
		ax12test->setPunch(punchL,punchH);
	}
	void setSlopes(){  //Change la variation de
		uint16_t cwSlope=32;
		uint16_t ccwSlope=32;
		serial.printflnDebug("Entrez CW_SLOPE (défaut=32)");
		serial.read(cwSlope);
		serial.printflnDebug("Entrez CCW_Slope(défaut=32)");
		serial.read(ccwSlope);
		ax12test->setSlopes(cwSlope,ccwSlope);
		
	}
};

#endif /* ACTUATORSMGR_HPP */
