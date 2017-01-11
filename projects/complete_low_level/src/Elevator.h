/**
 *
 * Elevator.h
 *
 */

#ifndef ARM_ELEVATORMGR_H
#define ARM_ELEVATORMGR_H

#include "stm32f4xx.h"
#include "safe_enum.hpp"
#include "utils.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"

/**
 *
struct dir_def {
	enum type {
		UP, DOWN
	};
};
 

typedef safe_enum<dir_def> Direction;
*/

class Elevator{
private:
	int dir;
	void setDirection(dir);
public:
	Elevator(void);
	void initTimer();
	void initPWM();
	void initLED();
	void switchDir();
	void run();
};


#endif //ARM_ELEVATORMGR_H
