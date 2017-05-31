#include <Arduino.h>
#include "../include/Motor.h"
#include "../lib/Singleton.hpp"

int main(){
    Motor & motor=Singleton::Instance();
    Serial1.begin(115200);
    Serial2.begin(9600);
}