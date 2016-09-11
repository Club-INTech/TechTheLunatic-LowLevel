
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


