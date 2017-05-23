/**
 * Motor.h
 *
 */

#ifndef __MOTOR_h__
#define __MOTOR_h__

#include <Uart.hpp>
#include "stm32f4xx.h"
#include "safe_enum.hpp"
#include "utils.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"

extern Uart<1> serial;

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

#endif
