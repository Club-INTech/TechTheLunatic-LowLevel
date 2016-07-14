/**
 * BinaryMotorMgr.hpp
 *
 */

#ifndef __BINARYMOTORMGR_h__
#define __BINARYMOTORMGR_h__

#include "stm32f4xx.h"
#include "safe_enum.hpp"
#include "utils.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "Singleton.hpp"
#include "delay.h"



class BinaryMotorMgr : public Singleton<BinaryMotorMgr> {

public:
	BinaryMotorMgr();

	void runAxisLeft();
	void runAxisRight();
	void stopAxisLeft();
	void stopAxisRight();

	void runForwardLeft();
	void runBackwardLeft();
	void runForwardRight();
	void runBackwardRight();

	void stopLeftDoor();
	void stopRightDoor();

	void setRightDoorOpening(bool);
	void setLeftDoorOpening(bool);
	void setRightDoorClosing(bool);
	void setLeftDoorClosing(bool);

	bool isRightDoorOpening();
	bool isLeftDoorOpening();
	bool isRightDoorClosing();
	bool isLeftDoorClosing();

	void manageBlockedDoor();
	bool isRightDoorBlocked();
	bool isLeftDoorBlocked();
};

#endif
