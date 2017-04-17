//
// Created by tic-tac on 13/01/17.
//

#ifndef ARM_ELEVATOR_H
#define ARM_ELEVATOR_H



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
	void run(void);
	void stop(void);
	void initialize(void);
};
#endif //ARM_ELEVATOR_H
