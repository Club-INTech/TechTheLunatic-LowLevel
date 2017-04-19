#include "Elevator.h"


/**
 * Gestion de l'ascenseur
 *
 * Pins du moteur:
 * Sens: PE13 en OUT
 * PWM: PB15 (TIM12_CH2)
 *
 * SENS: en vue du dessus: PE13
 * Trigonométrique:1 (à vérifier empiriquement)
 * Antitrigonomètrique:0
 *
 * @author Ug
 **/


Elevator::Elevator(void) {}


/*       __________________
       *|				   |*
       *|  Initialisation  |*
       *|__________________|*
*/

//Initialise le timer : configure le TIMER12
void Elevator::initTimer()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);      // Activation de l'horloge du TIMER12

    TIM_TimeBaseInitTypeDef timTimeBaseInitTypeDef;            // On définit la variable qu'on va utiliser
    TIM_TimeBaseStructInit(&timTimeBaseInitTypeDef);           // Initialise avec les valeurs par défaut

    uint16_t prescaler = (uint16_t)((SystemCoreClock / 2) / 100000) - 1; // on associe une fréquence de 1KHz au PWM
    // donc le PWM change toutes les ms

    timTimeBaseInitTypeDef.TIM_Period=10;                     // répétition du motif
    // NB: le PWM a pour valeur x<10 et on peut déduire un taux d'occupation sur la période
    timTimeBaseInitTypeDef.TIM_ClockDivision=0;                // on divise par 1 dans la formule (cf Motor.cpp)

    timTimeBaseInitTypeDef.TIM_Prescaler=prescaler;            // cf. Motor.cpp
    timTimeBaseInitTypeDef.TIM_CounterMode=TIM_CounterMode_Up; // compte en incrémentant

    TIM_TimeBaseInit(TIM12, &timTimeBaseInitTypeDef);          // actualisation des paramètres du TIMER
}

//Initialise le PWM
void Elevator::initPWM()
{
    TIM_OCInitTypeDef timOcInitTypeDef;                        // on définit la variable qu'on va utiliser

    timOcInitTypeDef.TIM_OCMode=TIM_OCMode_PWM1;               // mode du PWM : normal (non CTC)
    timOcInitTypeDef.TIM_OutputState=TIM_OutputState_Enable;   // relie la sortie de timer à la ligne physique
    timOcInitTypeDef.TIM_OCPolarity=TIM_OCPolarity_High;       // influe sur la forme du PWM
    timOcInitTypeDef.TIM_Pulse=0;                              // PWM initial nul
    TIM_OC2Init(TIM12, &timOcInitTypeDef);                     // initialisation de la structure pointée par &timOcInitTypeDef sur le canal 2 de TIM12

    TIM_OC2PreloadConfig(TIM12, TIM_OCPreload_Enable);         // active le registre périphérique de précharge du TIMER sur CCR2
    // (registre qui détermine quand on change la valeur pour le PWM)
    TIM_ARRPreloadConfig(TIM12, ENABLE);                       // active le registre périphérique de précharge du TIMER sur ARR
    // (registre qui contient la valeur à laquelle le TIMER se recharge)

    TIM_Cmd(TIM12, ENABLE);                                    // Active le TIMER
}

//Initialise les pins
void Elevator::initPins(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); // activation de l'horloge du port E
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); // activation de l'horloge du port B

    GPIO_InitTypeDef gpioPinInitStruct;                  // on définit la variable qu'on va utiliser
    GPIO_StructInit(&gpioPinInitStruct);                 // initialisation avec les valeurs par défaut

    //pin de sens
    gpioPinInitStruct.GPIO_Pin=GPIO_Pin_13;              // le canal de numéro 13 est initialisé (PE13 = pin de sens)
    gpioPinInitStruct.GPIO_Mode=GPIO_Mode_OUT;           // mode output
    GPIO_Init(GPIOE, &gpioPinInitStruct);                // actualisation des paramètres du port E

    //pin de PWM
    gpioPinInitStruct.GPIO_Pin=GPIO_Pin_15;              // le canal de numéro 15 est initialisé (PB15 = pin de pwm)
    gpioPinInitStruct.GPIO_Mode=GPIO_Mode_AF;            // Alternative Function
    gpioPinInitStruct.GPIO_PuPd=GPIO_PuPd_UP;            // si le signal est indéterminé et plutôt élevé, il sera de valeur Vcc
    gpioPinInitStruct.GPIO_OType=GPIO_OType_PP;          // le type en output est Push-Pull
    gpioPinInitStruct.GPIO_Speed=GPIO_Speed_100MHz;      // fréquence maximale en output
    GPIO_Init(GPIOB, &gpioPinInitStruct);                // actualisation des paramètres du port B

    GPIO_PinAFConfig(GPIOB,                              // port utilisé pour la fonction alternative
                     GPIO_PinSource15,                   // pin qui délivre la fonction alternative
                     GPIO_AF_TIM12);                     // connecte les pins du TIMER à la fonction alternative AF9

    GPIO_ResetBits(GPIOE,GPIO_Pin_13);                     // Initialise le moteur dans le sens montant
}

//Initialisation utilisée dans le main
void Elevator::initialize(void){
    initPins();
    initTimer();
    initPWM();
}


/*       _____________
       *|			  |*
       *|  Fonctions  |*
       *|_____________|*
*/

// Change la direction dans le sens souhaité(UP ou DOWN)
void Elevator::setSens(Sens sensToSet) {
    if (sensToSet==UP){
        GPIO_ResetBits(GPIOE, GPIO_Pin_13);   // On passe à 1 la valeur du bit de sens PE13 (sens trigo)
    }
    else if (sensToSet==DOWN){
        GPIO_SetBits(GPIOE, GPIO_Pin_13); // On passe à 0 la valeur du bit de sens PE13 (sens antitrigo)
    }
}

// Tourne dans le sens de sens (mouvement non asservi)
void Elevator::run(int8_t pwm) {
        TIM12->CCR2=pwm;           // CCR2 prend la valeur minimale entre 255 et le pwm
}

// Stoppe le mouvement de l'ascenseur
void Elevator::stop(void){
    TIM12->CCR2=0;

}


