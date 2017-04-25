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

#define vCaleDesc 45
#define vCaleUp 100


//Positions des AX12

//Pour la Pelleteusatron 3000

#define brapeldep 127
#define brapelram 113

#define brapelmoit 170
#define brapelrel 300
#define pospelinitG 300
#define pospeltientG 116
#define pospelmoitG 150
#define pospeldeliG 0
#define pospelinitD 0
#define pospeltientD 184
#define pospelmoitD 150
#define pospeldeliD 300

//Pour les attrape-modules

#define AMdebG 185
#define AMmidG 135
#define AMfinG 59
#define AMdebD 115
#define AMmidD 170
#define AMfinD 257 //avant: 232

//Les cale-modules
#define CaleHautG 120
#define CaleReposG 200
#define CaleBasG 235
#define CaleHautD 200
#define CaleReposD 135
#define CaleBasD    90

//Largue modules
#define LargueRepos 132
#define LarguePousse 175

/**
 * Classe de gestionnaire d'AX12, comporte toutes les fonctions relatives aux actions effectuées par les ax12 ou groupes d'ax12
 *
 */
class ActuatorsMgr : public Singleton<ActuatorsMgr>
{
private:
    typedef Uart<2> serial_ax;  // On utilise le port série 2 de la stm32
    AX<serial_ax>* ax12test;    // ax12 de test
    AX<serial_ax>* ax12brapel;  //objet gérant les deux AX12 des bras
    AX<serial_ax>* ax12pelG;     //ax12 pour la pelle de la pelleteuse
    AX<serial_ax>* ax12pelD;

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
        ax12pelG = new AX<serial_ax>(2,0,1023);
        ax12pelG->init();
        ax12pelD = new AX<serial_ax>(8,0,1023);
        ax12pelD->init();

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
//suuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuus - xavier
    ~ActuatorsMgr()
    {
        delete(ax12test);
        delete(ax12brapel);
        delete(ax12pelG);
        delete(ax12pelD);
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

        serial.printfln("Brancher l'AX12 gauche de la pelle");
        serial.read(i);
        ax12pelG->initIDB(2);
        ax12pelG->init();
        serial.printfln("Brancher l'AX12 droit de la pelle");
        serial.read(i);
        ax12pelD->initIDB(8);
        ax12pelD->init();
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
        ax12brapel->changeSpeed(10);
        ax12brapel->goTo(brapelrel);
        serial.printflnDebug("done");
    }

    void braPelDeplie() // déplie les bras de la pelle
    {
        serial.printflnDebug("Baisse les bras");
        ax12brapel->changeSpeed(10);
        ax12brapel->goTo(brapeldep);
        serial.printflnDebug("done");
    }
    void braPelRam() // déplie les bras de la pelle
    {
        serial.printflnDebug("Baisse les bras");
        ax12brapel->changeSpeed(10);
        ax12brapel->goTo(brapelram);
        serial.printflnDebug("done");
    }

    void braPelMoit()
    {
        serial.printflnDebug("Leve les bras mais pas trop");
        ax12brapel->changeSpeed(20);
        ax12brapel->goTo(brapelmoit);
        serial.printflnDebug("done");
    }

    void pelleInit()
    {
        serial.printflnDebug("Pelle va au début");
        ax12pelG->changeSpeed(50);
        ax12pelD->changeSpeed(50);
        ax12pelG->goTo(pospelinitG);
        ax12pelD->goTo(pospelinitD);
        serial.printflnDebug("done");
    }

    void pelleMoit()
    {
        serial.printflnDebug("Pelle tient boules");
        ax12pelG->changeSpeed(40);
        ax12pelD->changeSpeed(40);
        ax12pelG->goTo(pospelmoitG);
        ax12pelD->goTo(pospelmoitD);
        serial.printflnDebug("done");
    }

    void pelleTient(){
        serial.printflnDebug("pos de maintient des boules");
        ax12pelG->changeSpeed(40);
        ax12pelD->changeSpeed(40);
        ax12pelG->goTo(pospeltientG);
        ax12pelD->goTo(pospeltientD);
        serial.printflnDebug("done");
    }

    void pelleLib()
    {
        serial.printflnDebug("Pelle jete boules");
        ax12pelG->changeSpeed(50);
        ax12pelD->changeSpeed(50);
        ax12pelG->goTo(pospeldeliG);
        ax12pelD->goTo(pospeldeliD);
        serial.printflnDebug("done");
    }

/*			 ___________________
 * 		   *|                   |*
 *		   *|  Attrappe Module  |*
 *		   *|___________________|*
 */

    void moduleDeb(int cote)
    {
        AMG->changeSpeed(40);
        AMD->changeSpeed(40);
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
        AMG->changeSpeed(20);
        AMD->changeSpeed(20);
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
        AMG->changeSpeed(20);
        AMD->changeSpeed(20);
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
        CMD->changeSpeed(vCaleUp);
        CMD->goTo(CaleHautD);
    }
    void caleMidD(){
        CMD->changeSpeed(vCaleUp);
        CMD->goTo(CaleReposD);
    }
    void caleBasD(){
        CMD->changeSpeed(vCaleDesc);
        CMD->goTo(CaleBasD);
    }

    void caleHautG(){
        CMG->changeSpeed(vCaleUp);
        CMG->goTo(CaleHautG);
    }
    void caleMidG(){
        CMG->changeSpeed(vCaleUp);
        CMG->goTo(CaleReposG);
    }
    void caleBasG(){
        CMG->changeSpeed(vCaleDesc);
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
        LM->changeSpeed(18);
        LM->goTo(LarguePousse);
    }
    void larguePousselent(){
        LM->changeSpeed(12);
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
/*
    void testSync1(){
        serial.printflnDebug("sync 1");
        ax12pelG->syncWrite(2,8,0,300);
        serial.printflnDebug("done");
    }
    void testSync2(){
        serial.printflnDebug("sync 2");
        ax12pelG->syncWrite(2,8,300,0);
        serial.printflnDebug("done");
    }
    */
};

#endif /* ACTUATORSMGR_HPP */
