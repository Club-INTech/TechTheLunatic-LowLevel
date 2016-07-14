/*
*	Code des arduinos contrôlant l'allumage des DELs des yeux du robot.
*	Ces arduinos reçoivent les instructions d'affichage via une liaison série.
*
*	@author Sylvain Gaultier
*	@date 22/02/2016
*/

/*
Protocole :
premier octet : ID
77 octets suivants : data 
79eme octet : luminosité (entre 0 et 127)

ensuite : 
FLIP_ORDER permet d'actualiser l'affichage
*/



// Indique s'il s'agit du code de l'oeuil gauche ou droit
#define DROIT



#ifdef GAUCHE
	#define ID 128
#else
	#ifdef DROIT
	#define ID 129
	#endif
#endif


/* Dimention des matrices de DELs */
#define NB_LIGNES	7
#define NB_COLONNES	11


/* ***** PIN MAPPING *****/
//LIGNES	=> côté positif
//COLONNES	=> côté négatif

//Transforme un numéro de colonne (de 0 à NB_COLONNE - 1) en pin de l'arduino
uint8_t pinColonne(uint8_t nb)
{
	static uint8_t correspondance[NB_COLONNES] = { 9,10,12,11,13,14,15,16,17,19,18 };
	return correspondance[nb];
}

//Transforme un numéro de ligne (de 0 à NB_LIGNE - 1) en pin de l'arduino
uint8_t pinLigne(uint8_t nb)
{
	static uint8_t correspondance[NB_LIGNES] = { 8,7,6,5,4,3,2 };
	return correspondance[nb];
}


/*
Tableau contant la commande en luminosité des DELs (0 = éteint ; 1 = allumé)
Il y a deux tableaux de LIGNES*COLONNES cases, on utilise l'un en lecture quand l'autre est utilisé en écriture
Ainsi on affiche un premier tableau tout en en recevant un nouveau que l'on enregistre dans le second tableau.
Pour mémoire : data[numTableau][ligne * NB_COLONNES + colonne]
*/
volatile bool data[2][NB_LIGNES * NB_COLONNES];


/*
Indique quel est le tableau de lecture.
Le tableau de lecture est data[tabLecture]
Le tableau d'écriture est data[1 - tabLecture]
*/
volatile uint8_t tabLecture = 0;

/* Durée totale nécésaire à l'affichage d'une ligne d'une image */
#define PERIODE		1000		//en µs

#define FLIP_ORDER 255

/* Délais "DELs allumées" et "DELs éteintes" durant un cycle */
volatile uint16_t delaiON;
volatile uint16_t delaiOFF;

/* Fonction d'actualisation des valeurs des délais en fonction de l'intensité voulue */
void actualiserDelais(uint16_t intensite)
{
	if (intensite <= 127)
	{
		delaiON = PERIODE * ((float)intensite / 127);
		delaiOFF = PERIODE * (1 - (float)intensite / 127);
	}
}

/* Baudrate (débit en octets/seconde) pour la réception des instructions */
#define BAUDRATE 38400

/*
***** Note relative aux framerate/débits/synchronisations *****
L'émetteur décide du framerate en envoyant un byte à 255 pour déclencher l'affichage d'une nouvelle image.
Il convient de choisir un baudrate permettant d'envoyer 3000 octets entre deux frames.
La durée nécéssaire à l'affichage d'une image ('PERIODE') doit être petite devant la durée séparant deux frames.
*/

void setup()
{
	Serial.begin(BAUDRATE);
	for (int i = 0; i < NB_LIGNES; i++)
	{
		pinMode(pinLigne(i), OUTPUT);
	}
	for (int i = 0; i < NB_COLONNES; i++)
	{
		pinMode(pinColonne(i), OUTPUT);
	}
	for (int j = 0; j < NB_LIGNES * NB_COLONNES; j++)
	{
		data[0][j] = true;
	}
	for (int j = 0; j < NB_LIGNES * NB_COLONNES; j++)
	{
		data[1][j] = false;
	}

	actualiserDelais(50);
}

// Boucle affichant l'image issue de la partie 'lecture' du tableau 'data'
void loop()
{
	static uint8_t ligne = 0, colonne = 0;
	static unsigned long beginTime = 0;

	for (ligne = 0; ligne < NB_LIGNES; ligne++)
	{
		digitalWrite(pinLigne(ligne), HIGH);
		for (colonne = 0; colonne < NB_COLONNES; colonne++)
		{
			if (data[tabLecture][ligne * NB_COLONNES + colonne] > 0)
				digitalWrite(pinColonne(colonne), HIGH);
		}

		beginTime = micros();
		while (micros() < beginTime + delaiON)
		{
			if (Serial.available())
			{
				serialEvent();
			}
		}

		for (colonne = 0; colonne < NB_COLONNES; colonne++)
		{
			digitalWrite(pinColonne(colonne), LOW);
		}

		digitalWrite(pinLigne(ligne), LOW);

		beginTime = micros();
		while (micros() < beginTime + delaiOFF)
		{
			if (Serial.available())
			{
				serialEvent();
			}
		}
	}
}


// Lecture du buffer de réception série. Met à jour la partie 'écriture' du tableau 'data'.
void serialEvent()
{
	static uint8_t indice = 0, ligne, colonne, luminosite = 25;
	static int byte;
	static bool lectureTrameEnCours = false;

	byte = Serial.read();

	if (byte == FLIP_ORDER)	// Ordre de "flip screen"
	{
		actualiserDelais(luminosite);
		tabLecture = 1 - tabLecture;
	}
	else if (byte >= 128)	// Indicateur de début de trame
	{
		if (byte == ID)	// Vérification du destinataire
		{
			lectureTrameEnCours = true;	// La trame nous est destiné, on commence donc la lecture.
			indice = 0;
		}
	}
	else if (lectureTrameEnCours)
	{
		if (indice == 77)	// Lecture du dernier octet de la trame, correspondant à la luminosité.
		{
			luminosite = byte;
			lectureTrameEnCours = false;	// On a terminé la lecture de la trame
		}
		else
		{
			data[1 - tabLecture][indice] = byte;
			indice++;
		}
	}
}