#include <ax12.h>

//Initialisation port ARDUINO

const unsigned int output_IR=0; // Capteur IR en Analog pin 0
const unsigned int commandeFusee = 9; //Commande transistor pour rocket

//Initialisation variable de temps
const unsigned int t = 90 ; //condition pour le while
unsigned int val=0; //valeur du capteur IR

AX12 pelle(0); // Adresse de l'AX12 (ici 0)

void setup() {
  
  //Initialisation port serie
  
  //Serial.begin(9600); // baud rate

  //Initialisation AX12
  
  AX12::init(1000000);  // debit AX12 1Mb/s
  pelle.writeInfo(TORQUE_ENABLE, 1);
  pelle.writeInfo(MAX_TORQUE,500);
  

  
  //Initialisation moteur fusee
  
  pinMode(commandeFusee, OUTPUT); // pin 9 en sortie
  digitalWrite(commandeFusee, LOW); // initialise à 0

}

void launch() //active le moteur pendant X sec pour retirer la goupille
{
  digitalWrite(commandeFusee, HIGH);
  delay(3000);                                                   //A DEFINIR EMPIRIQUEMENT
  digitalWrite(commandeFusee, LOW);
}

void pelleteuse() //active l'AX12                              timer  ARBITRAIRE A DEFINIR EMPIRIQUEMENT
{
   // temps pour que les boules arrive 
  angle(pelle,120);
  delay(1000); //temps que les boules tombe 
  angle(pelle,150); //realise un dernier acoup 
  delay(500);
  angle(pelle,0); //reviens a la position de départ
}

byte angle(AX12 ax, int a)
{
 return ax.writeInfo(GOAL_POSITION, map(a,0,300,0,1023));
}

void loop() {

  for(int id=0;id <= 253; id++)
  {
    pelle.id = id;
    if(pelle.writeInfo(AX_LED,1) == 0)
      break;
  }
  
  while( (millis()/1000) < t ) //temps que 90 sec ne s'est pas ecoule.
  {
    val = analogRead(output_IR);
    
    if( val>400 && millis() < 82000) //Si obstacle près du capteur IR et que le temps est inferieur a 82sec
    {
      pelleteuse();
    }
    delay(1000);
  }
   launch();
   delay(100000);
}
