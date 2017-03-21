#! /bin/bash
sudo chmod 777 /dev/ttyUSB*
sudo screen /dev/ttyUSB* 115200
wait
sudo killall screen
