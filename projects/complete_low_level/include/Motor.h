/**
 * Moteur.cpp
 *
 * Classe de gestion d'un moteur (PWM, direction...)
 *
 * R�capitulatif pins utilis�es pour contr�ler les deux moteurs :
 *
 * Gauche :
 * 	-pins de sens : ??
 * 	-pin de pwm : ??
 * Droit :
 * 	-pins de sens : ??
 * 	-pin de pwm : ??
 *
 */

#ifndef COMPLETE_LOW_LEVEL_MOTOR_H
#define COMPLETE_LOW_LEVEL_MOTOR_H

#define LEFT_WHEEL_DIR 0
#define LEFT_WHEEL_PWM 0
#define RIGHT_WHEEL_DIR 0
#define RIGHT_WHEEL_PWM 0

#include "../lib/safe_enum.hpp"

struct direction_def {
    enum type {
        BACKWARD, FORWARD
    };
};
struct side_def {
    enum type {
        LEFT, RIGHT
    };
};

typedef safe_enum<direction_def> Direction;
typedef safe_enum<side_def> Side;

class Motor {
private:
    Side side;
    Direction direction;
    void setDirection(Direction);

public:
    Motor(Side);
    int16_t pwm;
    static void initPWM();
    void run(int16_t);

    int getPWM();

    char getDir();

    char getSide();
};

#endif //COMPLETE_LOW_LEVEL_MOTOR_H
