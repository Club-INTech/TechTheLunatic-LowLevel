@author : Rémi Dulong

--------------------------------------------------------------------

		Installation (bas niveau)

--------------------------------------------------------------------

1) Lancez le script installation_debian.sh EN TANT QUE ROOT

2) Lancez le script installation_clion.sh EN TANT QU'UTILISATEUR !

3) Enjoy


--------------------------------------------------------------------
			
		Script de Flash de la STM32

--------------------------------------------------------------------

1) Vérifiez que vous avez bien installé les dépendances :

	- stlink
	- arm-gcc-none-eabi

2) Compilez le programme sous CLion, en paramétrant le "Build Output Path"
   vers le dossier low_level_flash/build

3) Branchez la STM32 à votre PC

4) Lancez ./flash.sh

5) Enjoy





--------------------------------------------------------------------
