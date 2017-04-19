//
// Created by tic-tac on 13/01/17.
//

#ifndef ARM_ELEVATOR_H
#define ARM_ELEVATOR_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "safe_enum.hpp"
#include "utils.h"

class Elevator{
public:
	enum Sens{
		UP, DOWN
	};
	Elevator(void);
	void initTimer(void);
	void initPWM(void);
	void initPins(void);
	void setSens(Sens);
	void run(int8_t);
	void stop(void);
	void initialize(void);
};
#endif //ARM_ELEVATOR_H
