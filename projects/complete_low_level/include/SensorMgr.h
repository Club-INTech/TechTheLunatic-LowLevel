#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "capteur_srf05.hpp"
#include "Singleton.hpp"
#include "MotionControlSystem.h"
#include <Uart.hpp>

extern Uart<1> serial;

class SensorMgr : public Singleton<SensorMgr>
{
public:
	SensorMgr();


	int getSensorDistanceARD();

	bool isJumperOut() const;

	void refresh(MOVING_DIRECTION);

	void sensorInterrupt(int);


	void ARDInterrupt();



private:

	CapteurSRF ultrasonARD;

	unsigned int refreshDelay;
	unsigned int currentTime;
	unsigned int lastRefreshTime;
};


#endif
