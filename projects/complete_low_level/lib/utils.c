#include "utils.h"

/*
 * La fonction '%' du langage C ne renvoie pas le modulo au sens math�matique du terme.
 * En r�alit� pour (a % b) il fait ( ABS(a) % b)*SIGNE(a)
 * Voici donc pour votre plus grand plaisir, la fonction modulo math�matiquement CORRECTE.
 *
 * De rien, c'est gratuit.
 *
 * @author: Sylvain Gaultier
 * */

int modulo(int nombre, int modulo)
{
	if(modulo <= 0)
	{
		return -1;
	}
	else if(nombre >= 0 )
	{
		return nombre % modulo;
	}
	else
	{
		return modulo - ((-nombre) % modulo);
	}
}
