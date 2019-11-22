const int pinA = 12;
const int pinB = 8;
const int pinC = 5;
const int pinD = 3;
const int pinE = 2;
const int pinF = 11;
const int pinG = 6;
const int pinDP = 4;
const int pinD1 = 7;
const int pinD2 = 9;
const int pinD3 = 10;
const int pinD4 = 13;
int dpState = LOW;

const int segSize = 8;
const int noOfDisplays = 4;
const int noOfDigits = 10;

int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

int digits[noOfDisplays] = {
  pinD1, pinD2, pinD3, pinD4 
};

int digitMatrix[noOfDigits][segSize - 1] = {
// a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

const int pinSW = A2;
const int pinX = A0;
const int pinY = A1;

int pushed = 0;
int deAfisat = 0;
int swVal;
int currentSwVal = 0;
int lastSwVal = 0;
int xValue = 0;
int yValue = 0;

int digit = 0;

int minThreshold = 400;
int maxThreshold = 600;

bool joyMoved = false;

int pozitieX = 0;

// Functie pentru a afisa numarul
void displayNumber(int digit) {
  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }
}

// Functie pentru
  
// activate the display no. received as param
void showDigits(int num) {
  if(num>4)
  {
    for(int i = 0; i < noOfDisplays+1; i++)
    {
      digitalWrite(digits[i], LOW);
    }
  }
  else
  {
    digitalWrite(digits[num], HIGH);  
  }
}

long time = 0;
long debounce = 10000;

void setup() {
  //Declararea Joystick-ului
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  //Declararea 4x7-led display
  for (int i = 0; i < segSize - 1; i++)
  {
    pinMode(segments[i], OUTPUT);
  }
  for (int i = 0; i < noOfDisplays; i++)
  {
    pinMode(digits[i], OUTPUT);
  }

  //Indicatii Serial Monitor
  Serial.begin(9600);
}

void loop() {
  showDigits(4);
  //citim daca a fost apasat sau nu butonul
  currentSwVal = analogRead(pinSW);
  if(currentSwVal<1000) swVal = 0;
  else swVal = 1;
  if(swVal == 1 && deAfisat == 0)
  {
    pushed = 1 - pushed;
    delay(200);
  }

  deAfisat = swVal;
  if(pushed == 1)
  {
    //Daca a intrat in digit, atunci putem modifica valoarea y
      Serial.println("Y ACTIVAT");
      showDigits(pozitieX);
      yValue = analogRead(pinY);

      if(yValue < minThreshold && joyMoved == false)
      {
        if(digit>0) digit--;
        else digit = 9;
        joyMoved = true;
      }

      if(yValue > maxThreshold && joyMoved == false)
      {
        if(digit<9) digit++;
        else digit = 0;
        joyMoved = true;
      }

      if(yValue >= minThreshold && yValue <= maxThreshold)
      {
        joyMoved = false;
      }
      displayNumber(digit);
  }
  else
  {
    Serial.println("X = ON");
    showDigits(5);
    xValue = analogRead(pinX);
    if(xValue < minThreshold && joyMoved == false)
    {
      if(pozitieX > 0) pozitieX--;
      else pozitieX = 3;
      joyMoved = true;
    }
    if(xValue > maxThreshold && joyMoved == false)
    {
      if(pozitieX < 3) pozitieX++;
      else pozitieX = 0;
      joyMoved = true;
    }
    if(xValue >= minThreshold && xValue <= maxThreshold)
    {
      joyMoved = false;
    }
    
  }  
}
