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
	void initTimer(void);
	void initPWM(void);
	void initPins(void);
	void setSens(Sens);
	void run(void);
	void run(int);
	void stop(void);
	void initialize(void);
};
#endif //ARM_ELEVATOR_H
