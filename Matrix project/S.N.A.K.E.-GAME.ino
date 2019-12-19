// EEPROM including for highscore storing
#include <EEPROM.h>

/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |          8 x 8 LED MATRIX           |
 |_ _ _ _ _ _ DECLARATION _ _ _ _ _ _ _|
*/
#include <LiquidCrystal.h>
#define RS 7
#define E  6
#define D4 5
#define D5 4
#define D6 3
#define D7 2

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);


/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |         16 x 2 LCD DISPLAY          |
 |_ _ _ _ _ _ DECLARATION _ _ _ _ _ _ _|
*/

#include <LedControl.h>

// Data Input, Clock, Load and number of 8x8 MATRIX
LedControl matrix = LedControl(12, 11, 10, 1);

/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |      JOYSTICK PINS AND VALUES       |
 |_ _ _ _ _ _ DECLARATION _ _ _ _ _ _ _|
*/
#define xPin A0                 // x axis
#define yPin A1                 // y axis
#define swPin 8                 // switch button

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
int maximThreshold = 600;
bool joyMoved = false;


/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |     PIEZZO BUZZER NOTE AND PIN      |
 |_ _ _ _ _ _ DECLARATION _ _ _ _ _ _ _|
*/
#define buzzer_pin 13
#define NOTE_A4  440


/*_____________________________________
 |     GAMEPLAY VALUES AND STRUCTS     |
 |______SNAKE MOVEMENTS AND FOOD_______|
*/
struct Point {
  int row = 0, col = 0;
  Point(int row = 0, int col = 0) : row(row), col(col) {}
};

struct Coordinate {
  int x = 0, y = 0;
  Coordinate(int x = 0, int y = 0) : x(x), y(y) {}
};

Point snake;          // this point will receive a random value at the beginning for the head snake
Point food(-1, -1);       // this point has -1 and -1 because it does not appear on the screen yet
Coordinate joystickDefault(500, 500);

short snakeInitialLength = 3; // this is the starting level 
int snakeLen = snakeInitialLength;
int snakeSpeed = 500;     // This is the complexity of the game
int snakeDir = 0;       // When snake does not have a direction yet

// level thresholds
int incrLvl[] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 };
int temp = 0;
int maxim = incrLvl[0];

//global values for the game
int lives = 3;          // beginning of 3 lives
int score = 0;
int prevscore = 0;
int startingLevel = 0;
int level = 0;
const short messageSpeed = 5; // this is the message speed display at the end of the game


const short up = 1;
const short right = 2;
const short down = 3;
const short left = 4;

const int joystickThreshold = 160;

int gameboard[8][8] = {};


bool win = false;
bool gameOver = false;

//Comparing time
unsigned long Time;

/*-------------------------------------
  |       GAMEPLAY FUNCTIONS          |
  -------------------------------------
*/

// I initialize all the pins (Joystick, 8x8 LED MATRIX, LCD 16x2 Display and buzzer_pin Buzzer)
void beginning();

// the function that generates the random Food Point
void generateFood();

// this gets the Joystick values (X and Y)
void getJoystickValues();

// it calculates the Snake length and directions
void calculateSnake();

void endingGame();

// fix the edge when the snake meets the border (margin of the matrix) it spawns to the opposite side
void marginSolve();

/*---------------------------------------
  |       LCD DISPLAY FUNCTIONS         |
  ---------------------------------------
*/

void wait(int ms);

void selectMenu(int xValue);

void displayMenu(int number);

void option(int number);

void displaySetting(int number);

void setting(int number);

// -------------------------------------------------------------------------

/*---------------------------------------
  |       THE MAIN BOSS FUNCTIONS       |
  ---------------------------------------
*/
void setup() {
  beginning();
  //DEBUGGING
  Serial.begin(9600);
}

void loop() {
  xValue = analogRead(yPin);
  selectMenu(xValue);
}


// -------------------------------------------------------------------------

/*-------------------------------------
  |       GAMEPLAY FUNCTIONS          |
  -------------------------------------
*/
void beginning() {
  // Declare joystick PINS
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  // Declare buzzer_pin Buzzer PINS
  pinMode(buzzer_pin, OUTPUT);

  // LCD declaring
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("    WELCOME!    ");

  // 8x8 LED MATRIX declaring
  matrix.shutdown(0, false);    // it does not reset the first matrix
  matrix.setIntensity(0, 2);    // set the brightness of the leds to 2
  matrix.clearDisplay(0);     // clear display
  showSnakeMessage();


  // function that uses a seed to create a random Value from the analog INPUT A2 (values between 0 and 1023)
  randomSeed(analogRead(A2));

  // coordinates of the snake
  snake.row = random(8);
  snake.col = random(8);
}

void generateFood() {

  // First case: Test if the food is out of the matrix
  if (food.row == -1 || food.col == -1) {

    // if it's true, check if the snake is at its maximimum length (meaning you won :D :D :D)
    if (snakeLen >= 64) {
      win = true;
      return;
    }

    // unfortunately, if it's not true :( then spawn food to gib snack to the snake :d while there is space for it
    do {
      food.col = random(8);
      food.row = random(8);
    } while (gameboard[food.row][food.col] > 0);
  }
}

void getJoystickValues() {
  int previousDir = snakeDir;   // it stores the last direction

  long previousTime = millis();
  // We are increasing the speed based on the level
  while (millis() < previousTime + snakeSpeed) {
    // then we calculate the direction of the snake in an efficient way
    analogRead(yPin) < joystickDefault.y - joystickThreshold ? snakeDir = up : 0;
    analogRead(yPin) > joystickDefault.y + joystickThreshold ? snakeDir = down : 0;
    analogRead(xPin) < joystickDefault.x - joystickThreshold ? snakeDir = left : 0;
    analogRead(xPin) > joystickDefault.x + joystickThreshold ? snakeDir = right : 0;

    // and ignore any change by 180 degrees
    snakeDir + 2 == previousDir && previousDir != 0 ? snakeDir = previousDir : 0;
    snakeDir - 2 == previousDir && previousDir != 0 ? snakeDir = previousDir : 0;

    // blinking food
    matrix.setLed(0, food.row, food.col, millis() % 100 < 50 ? 1 : 0);
  }

}

void calculateSnake() {
  // we determine the snake Direction
  switch (snakeDir) {
  case up:
    snake.row--;
    marginSolve();
    matrix.setLed(0, snake.row, snake.col, 1);
    break;

  case right:
    snake.col++;
    marginSolve();
    matrix.setLed(0, snake.row, snake.col, 1);
    break;

  case down:
    snake.row++;
    marginSolve();
    matrix.setLed(0, snake.row, snake.col, 1);
    break;

  case left:
    snake.col--;
    marginSolve();
    matrix.setLed(0, snake.row, snake.col, 1);
    break;

  default: // if the snake is not moving, exit
    return;
  }

  // if there is a snake body segment, this will cause the lost of a life or the end of the game
  if (gameboard[snake.row][snake.col] > 1 && snakeDir != 0) {
    lives--;
    unrollSnake();
    snakeDir = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("YOU LOST A LIFE");
    wait(10);
    matrix.setLed(0, snake.row, snake.col, millis() % 100 < 50 ? 1 : 0);
    wait(2000);
  }
  // end of the game
  if (lives <= 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    GAME OVER    ");
    wait(10);
    lcd.setCursor(0, 1);
    prevscore = score;
    gameOver = true;
    return;
  }

  // check if the food was eaten
  if (snake.row == food.row && snake.col == food.col) {
    tone(buzzer_pin, NOTE_A4, 200);
    wait(10);
    noTone(buzzer_pin);

    // reset food
    food.row = -1;
    food.col = -1;

    // increment snake length
    snakeLen++;

    // increment all the snake body segments
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (gameboard[row][col] > 0) {
          gameboard[row][col]++;
        }
      }
    }

    if (gameOver || win) return;
    else {
      if (snakeLen == 10 || snakeLen == 20) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("YOU GAINED LIFE");
        wait(2000);
        showPlusLife();
        lives++;
      }
      if (snakeLen > maxim) {
        snakeSpeed -= 35;
        maxim = incrLvl[temp + 1];
        temp++;
      }

      //level = startingLevel;
      String firstRow = "Lives:";
      String secondRow = "Score:";
      firstRow = firstRow + lives + " Level:" + snakeLen;
      if (snakeLen < 10) score += 1;
      if (snakeLen >= 10) score += 4;
      if (snakeLen >= 30) score += 6;
      if (snakeLen >= 60) score += 10;

      if (score > EEPROMReadInt(0)) {
        EEPROMWriteInt(0, score);
      }
      secondRow = secondRow + score;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(firstRow);
      wait(10);
      lcd.setCursor(0, 1);
      lcd.print(secondRow);
      wait(10);
    }
  }

  // add new segment at the snake head location
  gameboard[snake.row][snake.col] = snakeLen + 1;

  // decrement all the snake body segments, if segment is 0, turn the corresponding led off
  for (int row = 0; row < 8; row++) {
    tone(buzzer_pin, NOTE_A4, 40);
    Time = millis();
    while (millis() < Time + 10) continue;
    noTone(buzzer_pin);
    for (int col = 0; col < 8; col++) {
      // if there is a body segment, decrement it's value
      if (gameboard[row][col] > 0) {
        gameboard[row][col]--;
      }

      // display the current pixel
      matrix.setLed(0, row, col, gameboard[row][col] == 0 ? 0 : 1);
    }
  }
}

void marginSolve() {
  snake.col < 0 ? snake.col += 8 : 0;
  snake.col > 7 ? snake.col -= 8 : 0;
  snake.row < 0 ? snake.row += 8 : 0;
  snake.row > 7 ? snake.row -= 8 : 0;
}

void endingGame() {
  if (gameOver || win) {
    // ending animation showing the score
    if (EEPROMReadInt(0) <= prevscore) showHighscore();
    wait(10);
    showScoreMessage(score);

    // ending animation showing the Game Over or WINNER
    if (gameOver) showGameOverMessage();
    else if (win) showWinMessage();

    // clear all the values and everything :(
    win = false;
    gameOver = false;
    snake.row = random(8);
    snake.col = random(8);
    food.row = -1;
    food.col = -1;
    snakeLen = snakeInitialLength;
    snakeDir = 0;
    lives = 3;
    score = 0;
    startingLevel = 0;
    level = 0;

    /*  resets the gameboard matrix
      cool function that resets the memory back to 0
      having (address, value to be filled, no. of bytes from the address)
    */
    memset(gameboard, 0, sizeof(gameboard[0][0]) * 8 * 8);
    matrix.clearDisplay(0);
  }
}

void unrollSnake() {
  // switch off the food LED
  matrix.setLed(0, food.row, food.col, 0);
  wait(800);

  // flash the screen 5 times
  for (int i = 0; i < 5; i++) {
    // invert the screen
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        matrix.setLed(0, row, col, gameboard[row][col] == 0 ? 1 : 0);
      }
    }
    wait(20);

    // invert it back
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        matrix.setLed(0, row, col, gameboard[row][col] == 0 ? 0 : 1);
      }
    }
    wait(50);
  }
  wait(600);

  for (int i = 1; i <= snakeLen; i++) {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (gameboard[row][col] == i) {
          matrix.setLed(0, row, col, 0);
          wait(100);
        }
      }
    }
  }
}


/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |       MESSAGES STORED IN            |
 |_ _ _ _ _ _A FLASH MEMORY_ _ _ _ _ _ |
*/
const PROGMEM bool plusLife[8][56] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool winMessage[8][56] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool highScore[8][56] = {
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0}
};

const PROGMEM bool snakeMessage[8][56] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool gameOverMessage[8][90] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool scoreMessage[8][58] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool digits[][8][8] = {
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 1, 1, 1, 0},
  {0, 1, 1, 1, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 1, 1, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 1, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 1, 0, 0},
  {0, 0, 1, 0, 1, 1, 0, 0},
  {0, 1, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0}
  }
};


/*_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 |           ANIMATIONSSSSS            |
 |_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|
*/
// scrolls the 'HIGHSCORE' message
void showHighscore() {
  // lambda function that captures all variable within scope by reference
  [&] {
    for (int d = 0; d < sizeof(highScore[0]) - 7; d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM aka FLASH MEMORY and displays it on the screen
          matrix.setLed(0, row, col, pgm_read_byte(&(highScore[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  // wait for joystick co come back
  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
    || analogRead(yPin) > joystickDefault.y + joystickThreshold
    || analogRead(xPin) < joystickDefault.x - joystickThreshold
    || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
  }
}

// scrolls the ' +1 <3 :D ' message
void showPlusLife() {
  // lambda function that captures all variable within scope by reference
  [&] {
    for (int d = 0; d < sizeof(plusLife[0]) - 7; d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM aka FLASH MEMORY and displays it on the screen
          matrix.setLed(0, row, col, pgm_read_byte(&(plusLife[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  // wait for joystick co come back
  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
    || analogRead(yPin) > joystickDefault.y + joystickThreshold
    || analogRead(xPin) < joystickDefault.x - joystickThreshold
    || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
  }
}

// scrolls the 'snake' message around the matrix
void showSnakeMessage() {
  [&] {
    for (int d = 0; d < sizeof(snakeMessage[0]) - 7; d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM and displays it on the screen
          matrix.setLed(0, row, col, pgm_read_byte(&(snakeMessage[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  // wait for joystick co come back
  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
    || analogRead(yPin) > joystickDefault.y + joystickThreshold
    || analogRead(xPin) < joystickDefault.x - joystickThreshold
    || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
  }

}

// scrolls the 'game over' message around the matrix
void showGameOverMessage() {
  [&] {
    for (int d = 0; d < sizeof(gameOverMessage[0]) - 7; d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM and displays it on the screen
          matrix.setLed(0, row, col, pgm_read_byte(&(gameOverMessage[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  // wait for joystick co come back
  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
    || analogRead(yPin) > joystickDefault.y + joystickThreshold
    || analogRead(xPin) < joystickDefault.x - joystickThreshold
    || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
  }

}

// scrolls the 'win' message around the matrix
void showWinMessage() {
  // lambda function that captures all variable within scope by reference
  [&] {
    for (int d = 0; d < sizeof(winMessage[0]) - 7; d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          // this reads the byte from the PROGMEM aka FLASH MEMORY and displays it on the screen
          matrix.setLed(0, row, col, pgm_read_byte(&(winMessage[row][col + d])));
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  // wait for joystick co come back
  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
    || analogRead(yPin) > joystickDefault.y + joystickThreshold
    || analogRead(xPin) < joystickDefault.x - joystickThreshold
    || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
  }

}

// scrolls the 'score' message with numbers around the matrix
void showScoreMessage(int score) {
  if (score < 0 || score > 99) return;

  // specify score digits
  int second = score % 10;
  int first = (score / 10) % 10;

  [&] {
    for (int d = 0; d < sizeof(scoreMessage[0]) + 2 * sizeof(digits[0][0]); d++) {
      for (int col = 0; col < 8; col++) {
        wait(messageSpeed);
        for (int row = 0; row < 8; row++) {
          if (d <= sizeof(scoreMessage[0]) - 8) {
            matrix.setLed(0, row, col, pgm_read_byte(&(scoreMessage[row][col + d])));
          }

          int c = col + d - sizeof(scoreMessage[0]) + 6; // move 6 px in front of the previous message

          // if the score is < 10, shift out the first digit (zero)
          if (score < 10) c += 8;

          if (c >= 0 && c < 8) {
            if (first > 0) matrix.setLed(0, row, col, pgm_read_byte(&(digits[first][row][c]))); // show only if score is >= 10 (see above)
          }
          else {
            c -= 8;
            if (c >= 0 && c < 8) {
              matrix.setLed(0, row, col, pgm_read_byte(&(digits[second][row][c]))); // show always
            }
          }
        }
      }

      // if the joystick is moved, exit the message
      if (analogRead(yPin) < joystickDefault.y - joystickThreshold
        || analogRead(yPin) > joystickDefault.y + joystickThreshold
        || analogRead(xPin) < joystickDefault.x - joystickThreshold
        || analogRead(xPin) > joystickDefault.x + joystickThreshold) {
        return; // return the lambda function
      }
    }
  }();

  matrix.clearDisplay(0);

  //  // wait for joystick co come back
  //  while (analogRead(yPin) < joystickDefault.y - joystickThreshold
  //          || analogRead(yPin) > joystickDefault.y + joystickThreshold
  //          || analogRead(xPin) < joystickDefault.x - joystickThreshold
  //          || analogRead(xPin) > joystickDefault.x + joystickThreshold) {}

}


/*-------------------------------------------------
  |         LCD MENU DISPLAY FUNCTIONS            |
  -------------------------------------------------
*/

void wait(int ms)
{
  Time = millis();
  while (millis() - Time <= ms)
  {
    continue;
  }
  return;
}

void selectMenu(int xValue) {

  if (xValue < minThreshold && joyMoved == false)
  {
    if (xPos > 1) xPos--;
    else xPos = 4;
    joyMoved = true;
  }

  if (xValue > maximThreshold&& joyMoved == false)
  {
    if (xPos < 4) xPos++;
    else xPos = 1;
    joyMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maximThreshold)
  {
    joyMoved = false;
  }
  displayMenu(xPos);

  //Reading the joystick pushbutton value to enter the submenu
  swVal = digitalRead(swPin);
  if (swVal == 0 && toShow == 0)
  {
    pushed = 1 - pushed;
    wait(200);
  }
  toShow = swVal;
  if (pushed == 1)
  {
    option(xPos);
    pushed = 0;
  }
  wait(10);
}

void displayMenu(int number) {
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
    lcd.print("Info");
    break;

  case 4:
    lcd.print("> Info");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Start game");
    break;

  default:
    break;
  }
  wait(10);
}

void option(int number) {
  lcd.clear();


  switch (number)
  {
    //Start game
  case 1:
  {
    while (pushed == 1)
    {
      int level = startingLevel;
      String firstRow = "Lives:";
      String secondRow = "Score:";
      firstRow = firstRow + lives + " Level:" + snakeLen;
      secondRow = secondRow + score;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(firstRow);
      wait(10);
      lcd.setCursor(0, 1);
      lcd.print(secondRow);
      wait(10);
      generateFood();
      getJoystickValues();
      calculateSnake();
      if (gameOver || win) {
        endingGame();
        toShow = 0;
        pushed = 0;
      }
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
        show += EEPROMReadInt(0);
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
      yPos = 3;
      Time = millis();
      while (millis() - Time < 100000)
      {
        yValue = analogRead(yPin);
        if (yValue < minThreshold && joyMoved == false)
        {
          if (yPos > 3) yPos--;
          else yPos = 8;
          joyMoved = true;
        }

        if (yValue > maximThreshold && joyMoved == false)
        {
          if (yPos < 8) yPos++;
          else yPos = 3;
          joyMoved = true;
        }

        if (yValue >= minThreshold && yValue <= maximThreshold)
        {
          joyMoved = false;
        }
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
          snakeLen = yPos;
          lcd.clear();
          lcd.setCursor(0, 0);
          String startLevel = "Snake length:";
          startLevel += yPos;
          lcd.print(startLevel);
          wait(10);
        }
        else return;
      }
      break;
    }
  case 4:
  {
    lcd.clear();
    toShow = 0;
    pushed = 0;
    Time = millis();
    while (millis() < Time + 100000)
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
        lcd.print(" Catalin Danila           S.N.A.K.E. ");
        wait(10);
        lcd.setCursor(0, 1);
        lcd.print(" github.com/Cata941823 UNIBUC ROBOTICS ");
        for (int positionCounter = 0; positionCounter < 44; positionCounter++) {
          // scroll one position left:
          lcd.scrollDisplayLeft();
        }
        wait(1000);
      }
      else return;
    }
    break;
  }
  default:
    break;
  }
}

void displaySetting(int number) {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (number)
  {
  case 1:
    lcd.print("> Snake length");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Insert name");
    break;

  case 2:
    lcd.print("> Insert name");
    wait(10);
    lcd.setCursor(0, 1);
    lcd.print("Snake length");
    break;
    
  default:
    break;
  }
  wait(10);
}

void setting(int number) {
  switch (number) {
  case 1:
  {
    snakeInitialLength = yPos;
    lcd.clear();
    lcd.setCursor(0, 0);
    String snakeInitialLength = "Snake length:";
    snakeInitialLength += yPos;
    snakeLen += yPos;
    lcd.print(snakeInitialLength);
    wait(10);
    break;
  }
  case 2:
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    String player = "Player name:";
    lcd.print(player);
    wait(10);
    break;
  }
  default:
    break;
  }
}

/*-------------------------------------------------
  |         EEPROM READ&WRITE FUNCTIONS            |
  -------------------------------------------------
*/


void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}

int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}
