// Ouverture du parasol (robot secondaire)
// Transistor Moteur PIN 7 ; jumper relié au PIN 8 ; led de "match en cours" pin 4 ; capteur de contact fin de course pin 3
// Moteur alimenté avec pile 9V et transistor polarisé amplificateur

bool done = false;
long int t_depart = 0;

#define PIN_LED     4
#define PIN_MOTOR   7
#define PIN_JUMPER  8
#define PIN_ENDSTOP 3

#define MATCH_DURATION 91000

void setup() {
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_JUMPER, INPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_ENDSTOP, INPUT);
   
}

void blinkLedMotherFucker(unsigned long delayOn, unsigned long delayOff){
  static unsigned long lastToggleTime = 0;
  static bool isOn = false;

  if(isOn && millis() - lastToggleTime >= delayOn && delayOff != 0){
    isOn = false;
    lastToggleTime = millis();
    digitalWrite(PIN_LED, LOW);
  }

  if(!isOn && millis() - lastToggleTime >= delayOff && delayOn != 0){
    isOn = true;
    lastToggleTime = millis();
    digitalWrite(PIN_LED, HIGH);
  }
}




void loop() {

  while(!digitalRead(PIN_JUMPER)){
    blinkLedMotherFucker(100, 100);
  }

  while(digitalRead(PIN_JUMPER)){
      blinkLedMotherFucker(500, 500);
  }

  unsigned long beginMatchTime = millis();

  while(millis()-beginMatchTime <= MATCH_DURATION){
    blinkLedMotherFucker(200, 600);
  }

  digitalWrite(PIN_MOTOR, HIGH);

  while(!digitalRead(PIN_ENDSTOP));

  digitalWrite(PIN_MOTOR, LOW);

  while(true){
    blinkLedMotherFucker(1,0);
  }

}
