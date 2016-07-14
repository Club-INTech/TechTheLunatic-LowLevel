#!/bin/bash
# Necessite st-link, arm-none-eabi-objcopy

echo ---Préparation du dossier build---
cd build/Debug
rm ll.bin
rm ll.hex
echo ---Extraction des .bin et .hex---
arm-none-eabi-objcopy -O binary complete_low_level ll.bin
arm-none-eabi-objcopy -O ihex complete_low_level ll.hex
echo ---Flash de la STM32---
sudo st-flash write ll.bin 0x8000000
echo ---Fin---
