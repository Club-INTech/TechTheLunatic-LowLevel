//
// Created by tic-tac on 13/01/17.
//

#ifndef ARM_ELEVATOR_H
#define ARM_ELEVATOR_H

enum Sens{
	UP, DOWN
};

class Elevator{
private:
	Sens sens;
public:
	Elevator(void);
	void initTimer();
	void initPWM();
	void initPins();
	void switchSens();
	void setSens(Sens);
	void run();
	void stop();
};
#endif //ARM_ELEVATOR_H
