/*
*	Code des arduinos contr�lant l'allumage des DELs des yeux du robot.
*	Ces arduinos re�oivent les instructions d'affichage via une liaison s�rie.
*
*	@author Sylvain Gaultier
*	@date 22/02/2016
*/

/*
Protocole :
premier octet : ID
77 octets suivants : data 
79eme octet : luminosit� (entre 0 et 127)

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
//LIGNES	=> c�t� positif
//COLONNES	=> c�t� n�gatif

//Transforme un num�ro de colonne (de 0 � NB_COLONNE - 1) en pin de l'arduino
uint8_t pinColonne(uint8_t nb)
{
	static uint8_t correspondance[NB_COLONNES] = { 9,10,12,11,13,14,15,16,17,19,18 };
	return correspondance[nb];
}

//Transforme un num�ro de ligne (de 0 � NB_LIGNE - 1) en pin de l'arduino
uint8_t pinLigne(uint8_t nb)
{
	static uint8_t correspondance[NB_LIGNES] = { 8,7,6,5,4,3,2 };
	return correspondance[nb];
}


/*
Tableau contant la commande en luminosit� des DELs (0 = �teint ; 1 = allum�)
Il y a deux tableaux de LIGNES*COLONNES cases, on utilise l'un en lecture quand l'autre est utilis� en �criture
Ainsi on affiche un premier tableau tout en en recevant un nouveau que l'on enregistre dans le second tableau.
Pour m�moire : data[numTableau][ligne * NB_COLONNES + colonne]
*/
volatile bool data[2][NB_LIGNES * NB_COLONNES];


/*
Indique quel est le tableau de lecture.
Le tableau de lecture est data[tabLecture]
Le tableau d'�criture est data[1 - tabLecture]
*/
volatile uint8_t tabLecture = 0;

/* Dur�e totale n�c�saire � l'affichage d'une ligne d'une image */
#define PERIODE		1000		//en �s

#define FLIP_ORDER 255

/* D�lais "DELs allum�es" et "DELs �teintes" durant un cycle */
volatile uint16_t delaiON;
volatile uint16_t delaiOFF;

/* Fonction d'actualisation des valeurs des d�lais en fonction de l'intensit� voulue */
void actualiserDelais(uint16_t intensite)
{
	if (intensite <= 127)
	{
		delaiON = PERIODE * ((float)intensite / 127);
		delaiOFF = PERIODE * (1 - (float)intensite / 127);
	}
}

/* Baudrate (d�bit en octets/seconde) pour la r�ception des instructions */
#define BAUDRATE 38400

/*
***** Note relative aux framerate/d�bits/synchronisations *****
L'�metteur d�cide du framerate en envoyant un byte � 255 pour d�clencher l'affichage d'une nouvelle image.
Il convient de choisir un baudrate permettant d'envoyer 3000 octets entre deux frames.
La dur�e n�c�ssaire � l'affichage d'une image ('PERIODE') doit �tre petite devant la dur�e s�parant deux frames.
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


// Lecture du buffer de r�ception s�rie. Met � jour la partie '�criture' du tableau 'data'.
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
	else if (byte >= 128)	// Indicateur de d�but de trame
	{
		if (byte == ID)	// V�rification du destinataire
		{
			lectureTrameEnCours = true;	// La trame nous est destin�, on commence donc la lecture.
			indice = 0;
		}
	}
	else if (lectureTrameEnCours)
	{
		if (indice == 77)	// Lecture du dernier octet de la trame, correspondant � la luminosit�.
		{
			luminosite = byte;
			lectureTrameEnCours = false;	// On a termin� la lecture de la trame
		}
		else
		{
			data[1 - tabLecture][indice] = byte;
			indice++;
		}
	}
}