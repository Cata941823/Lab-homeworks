const int passiveBuzzer = A0;
const int activeBuzzer = 11;
const int pushButton = 2;

int valPassiveBuzz = 0;
int valPushBtn;


unsigned long timpDeLaKnock;

void setup() {
  pinMode(passiveBuzzer, INPUT);
  pinMode(activeBuzzer, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  valPassiveBuzz = analogRead(passiveBuzzer);
  if (valPassiveBuzz != 0){
    timpDeLaKnock = millis();
    valPushBtn = digitalRead(pushButton);

    if (valPushBtn == 1){
      while(millis() - timpDeLaKnock < 5000) continue;
      if(millis() - timpDeLaKnock >= 5000) tone(activeBuzzer, 400);       
    }
    else noTone(activeBuzzer);
  } 
}
