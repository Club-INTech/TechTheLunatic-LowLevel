#!/bin/bash


#Installation du compilateu arm
if test /usr/bin/arm-none-eabi-gcc
then
 echo "arm-none-eabi-gcc  est déja installé sur cette machine"
else

	 echo "-------------------------------------------"
	 echo >2& "arm-none-eabi-gcc n'est pas installé."
	 echo  "Installation..."
	 apt-get install --yes --force-yes gcc-arm-none-eabi
	 echo "arm-none-eabi-gcc a été installé"
	 echo "-------------------------------------------"
 fi

# Installation de libusb
# (je voulais mettre un test mais comme c'est pas une commande,
# flemme. Au pire, ça réinstalle. YOLO.)
 apt-get install --yes --force-yes libusb-1.0-0-dev


#Installation de git
if test -d /usr/bin/git
then
 echo "Git est déja installé sur cette machine"
else


echo "-------------------------------------------"
	 echo >2& "git n'est pas installé."
	 echo  "Installation..."
	 apt-get install --yes --force-yes git
	 echo "git a été installé"
	 echo "-------------------------------------------"
 fi

#Installation de stlinki
if test -d /usr/bin/st-flash
then
	 echo "St-link est déja installé sur cette machine"
 else


	 echo "-------------------------------------------"
	 echo >2& "stlink n'est pas installé."
	 echo  "Installation..."
	 cd /opt
	 wget https://github.com/texane/stlink/archive/master.zip
	 unzip master.zip
	 rm master.zip
	 cd stlink*
	 mkdir build && cd build
	 cmake -DCMAKE_BUILD_TYPE=Debug ..
	 make
	 cp st-* /usr/bin
	 echo "stlink a été installé"
	 echo "-------------------------------------------"
 fi

#Installation de Clion


if test -d /opt/clion* 
then
 echo "CLion est déja installé sur cette machine"
else
	 echo "-------------------------------------------"
	 echo >2& "CLion n'est pas installé."
	 echo  "Installation..."
	 cd /opt
	 wget https://download.jetbrains.com/cpp/CLion-2016.2.1.tar.gz
	 tar xzvf CLion*
	 cd clion*/bin
	 rm /opt/Clion*.tar.gz
	 sh clion.sh
	 echo "CLion a été installé"
	 echo "-------------------------------------------"
fi

