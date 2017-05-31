//
// Created by tic-tac on 31/05/17.
//

#include <Arduino.h>
#include "../include/Motor.h"

Motor::Motor(Side s):
        side(s), direction(Direction::FORWARD){
    pwm=0;
    /*
     * CONFIGURATION DES PINS
     */

    if(side==Side::LEFT){
        pinMode(LEFT_WHEEL_DIR,OUTPUT);
        pinMode(LEFT_WHEEL_PWM,OUTPUT);
        //TODO:fréquences et résolutions des pwm
    }
    else if(side==Side::RIGHT){
        pinMode(RIGHT_WHEEL_DIR,OUTPUT);
        pinMode(RIGHT_WHEEL_PWM,OUTPUT);
    }

}

void Motor::run(int16_t pwmInput) {
    pwm=pwmInput;
    if(pwm>0){
        setDirection(Direction::FORWARD);
        pwm=min(pwm, 255);
    }
    else{
        setDirection(Direction::BACKWARD);
        pwm=min(-pwm, 255);
    }

    if(side==Side::LEFT){
        analogWrite(LEFT_WHEEL_PWM,pwm);
    }
    else if(side==Side::RIGHT){
        analogWrite(RIGHT_WHEEL_PWM,pwm);
    }
}

void Motor::setDirection(Direction dir) {
    direction=dir;
    if (side == Side::LEFT) {
        if (dir == Direction::BACKWARD) {
            digitalWrite(LEFT_WHEEL_DIR, LOW);
        } else {
            digitalWrite(LEFT_WHEEL_DIR, HIGH);
        }
    } else {
        if (dir == Direction::BACKWARD) {
            digitalWrite(RIGHT_WHEEL_DIR, LOW);
        } else {
            digitalWrite(RIGHT_WHEEL_DIR, HIGH);
        }
    }
}

int Motor::getPWM(){
    return pwm;
}

char Motor::getDir(){
    if(direction==Direction::FORWARD)
        return 'f';
    else
        return 'b';
}

char Motor::getSide(){
    if(side==Side::LEFT)
        return 'l';
    else
        return 'r';
}