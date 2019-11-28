#include <LiquidCrystal.h>

//LCD pins
const int RS = 2;
const int E = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
const int V0_PIN = A5;

//Joystick pins
const int xPin = A0;
const int yPin = A2;
const int swPin = 10;

//Joystick pushbutton values
int toShow = 0;
int pushed = 0;
int swVal = 0;

//Joystick axis values
int xValue = 1;
int yValue = 0;
int xPos = 1;
int yPos = 0;
int minThreshold = 400;
int maxThreshold = 600;
bool joyMoved = false;

//Comparing time
unsigned long Time;

//global values for the game
int lives = 3;
int score = 0;
int startingLevel = 0;

//lcd object declaration
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

//Function using millis() instead of wait()
void wait(int ms)
{
  Time = millis();
  while (millis() - Time <= ms)
  {
    continue;
  }
  return;
}

void selectMenu(int xValue)
{
  if (xValue < minThreshold && joyMoved == false)
  {
    if (xPos > 1) xPos--;
    else xPos = 3;
    joyMoved = true;
  }

  if (xValue > maxThreshold&& joyMoved == false)
  {
    if (xPos < 3) xPos++;
    else xPos = 1;
    joyMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maxThreshold)
  {
    joyMoved = false;
  }
  displayMenu(xPos);

  //Reading the joystick pushbutton value to enter the submenu
  swVal = digitalRead(swPin);
  if (swVal == 1 && toShow == 0)
  {
    pushed = 1 - pushed;
    wait(200);
  }
  toShow = swVal;
  if (pushed == 0)
  {
    option(xPos);
    pushed = 1;
  }
  wait(10);
}

void displayMenu(int number)
{
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (number)
  {
  case 1:
    lcd.print("> Start game");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Highscore");
    break;

  case 2:
    lcd.print("> Highscore");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Settings");
    break;

  case 3:
    lcd.print("> Settings");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Start game");
    break;

  default:
    break;
  }
  wait(10);
}

void option(int number)
{
  lcd.clear();

  switch (number)
  {
    //Start game
    case 1:
    {
      Time = millis();
      while (millis() - Time < 10000)
      {
        int level = startingLevel;
        String firstRow = "Lives:";
        String secondRow = "Score:";
        if (millis() - Time < 5000)
        {
          firstRow = firstRow + lives + " Level:" + level;
          score = 3 * level;
          secondRow = secondRow + score;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(firstRow);
          delay(10);
          lcd.setCursor(0, 1);
          lcd.print(secondRow);
          delay(10);
        }

        else if (millis() - Time < 10000 && millis() - Time >= 5000)
        {
          level += 1;
          firstRow = firstRow + lives + " Level:" + level;
          score = 3 * level;
          secondRow = secondRow + score;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(firstRow);
          delay(10);
          lcd.setCursor(0, 1);
          lcd.print(secondRow);
          delay(10);
        }
      }

      toShow = 0;
      pushed = 0;
      while (millis() - Time >= 10000)
      {
        swVal = digitalRead(swPin);
        if (swVal == 1 && toShow == 0)
        {
          pushed = 1 - pushed;
          delay(200);
        }

        toShow = swVal;
        if (pushed == 1)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(" !!!YOU WON!!!");
          delay(10);
          lcd.setCursor(0, 1);
          lcd.print("Click to exit.");
          delay(10);
        }
        else return;
      }
      break;
    }

    //Highscore
    case 2:
    {
      toShow = 0;
      pushed = 0;
      Serial.println(score);
      Time = millis();
      while (millis() - Time < 10000)
      {
        swVal = digitalRead(swPin);
        if (swVal == 1 && toShow == 0)
        {
          pushed = 1 - pushed;
          wait(200);
        }

        toShow = swVal;
        if (pushed == 1)
        {
          lcd.setCursor(0, 0);
          String show = "Highscore:";
          show += score;
          lcd.print(show);
          wait(10);
        }
        else return;
      }
      break;
    }

    //Settings
    case 3:
    {
      toShow = 0;
      pushed = 0;
      Time = millis();
      while (millis() - Time < 100000)
      {
        yValue = analogRead(yPin);
        if (yValue < minThreshold && joyMoved == false)
        {
          if (yPos > 0) yPos--;
          else yPos = 9;
          joyMoved = true;
        }

        if (yValue > maxThreshold&& joyMoved == false)
        {
          if (yPos < 9) yPos++;
          else yPos = 0;
          joyMoved = true;
        }

        if (yValue >= minThreshold && yValue <= maxThreshold)
        {
          joyMoved = false;
        }
        Serial.println(yPos);
        //citim daca a fost apasat butonul

        swVal = digitalRead(swPin);
        if (swVal == 1 && toShow == 0)
        {
          pushed = 1 - pushed;
          wait(200);
        }

        toShow = swVal;
        if (pushed == 1)
        {
          startingLevel = yPos;
          lcd.clear();
          lcd.setCursor(0, 0);
          String startLevel = "Start level:";
          startLevel += yPos;
          lcd.print(startLevel);
          wait(10);
        }
        else return;
      }
      break;
    }
    default:
      break;
  }
}

void setup() {
  //Declaration of Joystick
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  //PWM for V0 pin on LCD
  pinMode(V0_PIN, OUTPUT);
  analogWrite(V0_PIN, 10);

  //Declaring 2x16 grid for LCD
  lcd.begin(16, 2);

  //Serial monitor for debugging
  Serial.begin(9600);
}

void loop() {
  xValue = analogRead(xPin);
  selectMenu(xValue);
}
