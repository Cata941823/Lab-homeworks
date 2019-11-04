const int Pin[] = {3, 5, 6};
int ledVal[] = {0, 0, 0};

const int Pot[] = {A0, A1, A2};
int potVal[] = {0, 0, 0};

void setup() {
  for(int i = 0; i<3; i++)
  {
    pinMode(Pot[i], INPUT);
    pinMode(Pin[i], OUTPUT);
  }
}

void loop() {
  for(int i = 0; i<3; i++)
  {
    potVal[i] = analogRead(Pot[i]);
    ledVal[i] = map(potVal[i], 0, 1023, 0, 255);
  }
  setColor(ledVal[0], ledVal[1], ledVal[2]);
}

void setColor(int red, int green, int blue)
{
  analogWrite(Pin[0], red);
  analogWrite(Pin[1], green);
  analogWrite(Pin[2], blue);
}
