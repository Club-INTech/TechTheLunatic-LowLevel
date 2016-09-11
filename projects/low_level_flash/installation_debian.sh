#!/bin/bash


#Installation du compilateu arm
if test -f /usr/bin/arm-none-eabi-gcc
then
	echo "arm-none-eabi-gcc  est déja installé sur cette machine"
else

	echo "-------------------------------------------"
	echo >2& "arm-none-eabi-gcc n'est pas installé."
	echo  "Installation..."
	apt install --yes --force-yes gcc-arm-none-eabi
	echo "arm-none-eabi-gcc a été installé"
	echo "-------------------------------------------"
fi

# Installation de libusb
# (je voulais mettre un test mais comme c'est pas une commande,
# flemme. Au pire, ça réinstalle. YOLO.)
apt install --yes --force-yes libusb-1.0-0-dev


#Installation de git
if test -f /usr/bin/git
then
	echo "Git est déja installé sur cette machine"
else


	echo "-------------------------------------------"
	echo >2& "git n'est pas installé."
	echo  "Installation..."
	apt install --yes --force-yes git
	echo "git a été installé"
	echo "-------------------------------------------"
fi


#Installation de Cmake

if test -f /usr/bin/cmake
then 
	echo "Cmake est déja installé sur cette machine"
else
	echo "-------------------------------------------"
	echo >2& "cmake n'est pas installé."
	echo  "Installation..."
	apt install --yes --force-yes cmake
	echo "cmake a été installé"
	echo "-------------------------------------------"
fi




#Installation de stlink
if test -d /usr/bin/st-flash
then
	echo "St-link est déja installé sur cette machine"
else


	echo "-------------------------------------------"
	echo >2& "stlink n'est pas installé."
	echo  "Installation..."
	wget https://github.com/texane/stlink/archive/master.zip
	unzip master.zip
	rm master.zip
	cd stlink*
	mkdir build && cd build
	cmake -DCMAKE_BUILD_TYPE=Debug ..
	make
	cp st-* /usr/bin
	cd ../..
	sudo rm -r stlink-master
	echo "stlink a été installé"
	echo "-------------------------------------------"
fi
