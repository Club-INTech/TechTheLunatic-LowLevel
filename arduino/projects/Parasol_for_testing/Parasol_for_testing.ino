// Ouverture du parasol (robot secondaire)
// Transistor Moteur PIN 7 ; jumper relié au PIN 8 ; led de "match en cours" pin 4
// Moteur alimenté avec pile 9V et transistor polarisé amplificateur

bool done = false;
long int t_depart = 0;



void setup() {
  pinMode(5, OUTPUT);
  pinMode(8, INPUT);
  pinMode(4, OUTPUT);
  pinMode(6, INPUT);
  analogWrite(5, 0);
  digitalWrite(4, LOW);
  Serial.begin(9600);
}

void loop() {

  bool blink = false;
   if((digitalRead(8) || 1) && !done) { // On attend que le jumper soit mis en place (utile pour déterminer un front descendant, duh...) + config pour test
     while(digitalRead(8) || 1){
        Serial.println("Waiting");
        delay(5000); // Pour tests
        Serial.println("Launched!");
        break;  // Pour tests
      } // On attends le front descendant (enlevage du jumper)

     digitalWrite(4, HIGH); // On indique qu'il a compris que le match commence
     t_depart = millis();
     
     for(int i=0; i>=84; i++){ // 85 secondes
     
      delay(1000);  // Oui, c'est dégeulasse.
      blink = !blink;
      digitalWrite(4, blink);
     }

     for(int i=0; i>=19; i++){ // 5 dernières secondes
      delay(250);
      blink = !blink;
      digitalWrite(4, blink);
     }
     
      analogWrite(5, 255);
      while(42)
      {
        if(digitalRead(6))
        {
          analogWrite(5, 0);
          break;
        }
      }

     digitalWrite(4, LOW);
     done=true; // Empeche le système de se relancer
   }
  
}
