#! /bin/bash
sudo chmod 777 /dev/ttyUSB0
sudo screen /dev/ttyUSB0 115200
wait
sudo killall screen