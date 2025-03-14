#include <random>
#include <ESP32Servo.h>
#include <FastLED.h>
#include "AsyncTimer.h"

#define WIDTH 8
#define HEIGHT 5
#define NUM_LEDS ((WIDTH) * (HEIGHT))
#define LED_PIN 15
#define JOYSTICK_PIN_X 13
#define JOYSTICK_PIN_Y 12

////////////////////////////////////////////////////
// 
//  ENUMS
//
////////////////////////////////////////////////////
enum Direction {
  MOVE_STOP,
  MOVE_UP,
  MOVE_RIGHT,
  MOVE_DOWN,
  MOVE_LEFT
};

enum StateGame {
  GAME_WAITING,
  GAME_MOVING,
  GAME_LOOSE,
  GAME_WIN
};

enum StateLED {
  LED_BLACK = 0x000000,
  LED_WHITE = 0xFFFFFF,
  LED_RED = 0xFF0000,
  LED_GREEN = 0x00FF00,
  LED_BLUE = 0x0000FF,
  LED_CUSTOM = 0xFFFF00
};

////////////////////////////////////////////////////
//
// LEDS data
CRGB leds[NUM_LEDS + 1];

byte led_map_indexes[NUM_LEDS + 1] = {
   5,  0, 15, 10, 25, 20, 35, 30,
   6,  1, 16, 11, 26, 21, 36, 31,
   7,  2, 17, 12, 27, 22, 37, 32,
   8,  3, 18, 13, 28, 23, 38, 33,
   9,  4, 19, 14, 29, 24, 39, 34
};

static byte led_map(byte index) {
  return led_map_indexes[index];
}

static byte XY(byte x, byte y) {
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        int found_x = i % WIDTH; // Column index
        int found_y = i / WIDTH; // Row index
        if (found_x == x && found_y == y) {
          return led_map(i);
        }
    }
    return 0;
}

////////////////////////////////////////////////////
// 
//  GLOBAL VARIABLES
//
////////////////////////////////////////////////////
int snakeX = 0;
int snakeY = 0;
int appleX = 0;
int appleY = 0;

int directionX = 0; // (-1, 1, 0)
int directionY = 0; // (-1, 1, 0)
Direction direction;

////////////////////////////////////////////////////
// 
//  CLASSES
//
////////////////////////////////////////////////////
class SnakeItem {
public:
  CRGB* color;
  byte x;
  byte y;
};

class Snake {
public:
  byte quantity = 0;
  SnakeItem head;
  SnakeItem body[NUM_LEDS] = {};

  void init(byte x, byte y) {
    add(x,y);
    add(x-1, y);
    //add(x-2, y);
    //add(x-3, y);
    //add(x-4, y);

    head = body[0];
  }

  void add(byte x, byte y) {
    if (quantity >= NUM_LEDS-1)
      return;

    SnakeItem toAdd = create();
    toAdd.x = x;
    toAdd.y = y;
    body[quantity++] = toAdd;

    Serial.print("Snake item added ");
    Serial.print(x);
    Serial.print(" : ");
    Serial.println(y);
  }

  void add() {
    SnakeItem last = body[quantity-1];
    int x = last.x;
    int y = last.y;

    add(x,y);
    return;
    
    switch(direction) {
      case MOVE_LEFT:
        x++;
        break;
      case MOVE_RIGHT:
        x--;
        break;
      case MOVE_UP:
        y++;
        break;
      case MOVE_DOWN:
        y--;
        break;
    }

    add(x,y);
  }
  
  SnakeItem create() {
    return SnakeItem();
  }

  void move(byte x, byte y) {
    for (int i=quantity-1; i>=0; --i) {
      if (i > 0) {
        body[i].x = body[i-1].x;
        body[i].y = body[i-1].y;
      } else {
        body[i].x = x;
        body[i].y = y;
      }
    }
    
    return;
  }
};

class Matrix {
public:
  void all(StateLED value) {
    int index=0;
    while (index <= NUM_LEDS)
      leds[index++] = value;
  }

  void show(StateLED value = LED_WHITE) {
    int index=0;
    while (index <= NUM_LEDS)
      leds[index++] = value;
  }

  void hide() {
    all(LED_BLACK);
  }

  CRGB get(byte index) {
    return leds[led_map(index)];
  }

  CRGB get(byte x, byte y) {
    return leds[led_map(XY(x,y))];
  }
};

class Game {
public:
  Matrix matrix;
  // Direction direction;
  StateGame state;
  Snake snake;
};

Game game;
AsyncTimer asyncTimer;

////////////////////////////////////////////////////
// 
//  ASYNC LOOPS
//
////////////////////////////////////////////////////
void setDirection() {
  switch(direction) {
    case MOVE_LEFT:
      snakeX -= 1;
      if (snakeX < 0)
        snakeX = WIDTH-1;
      break;
    case MOVE_RIGHT:
      snakeX += 1;
      if (snakeX >= WIDTH)
        snakeX = 0;
      break;
    case MOVE_UP:
      snakeY -= 1;
      if (snakeY < 0)
        snakeY = HEIGHT-1;
      break;
    case MOVE_DOWN:
      snakeY += 1;
      if (snakeY >= HEIGHT)
        snakeY = 0;
      break;
    case MOVE_STOP:
      
    break;
  }
  // One-step moving
  // game.direction = MOVE_STOP;
  printDirection();
}

void setApple() {
  do {
    appleX = random(0,7);
    appleY = random(0,4);
  } while (appleX != snakeX && appleY != snakeY);

  leds[XY(appleX, appleY)] = LED_GREEN;
}

void loopGame() {
  setDirection();
  game.matrix.hide();

  game.snake.move(snakeX, snakeY);

  leds[XY(appleX, appleY)] = LED_GREEN;
  if (snakeX == appleX && snakeY == appleY) {
    game.snake.add();
    setApple();
  }

  for (byte i=0; i<game.snake.quantity; ++i) {
    byte x = game.snake.body[game.snake.quantity-i-1].x;
    byte y = game.snake.body[game.snake.quantity-i-1].y;
    byte mapped_index = XY(x, y);
    
    if (i==game.snake.quantity-1) {
      leds[mapped_index] = LED_RED;
    } else {
      leds[mapped_index] = LED_CUSTOM;
    }
  }

  FastLED.show();

  printPosition();
}

void loopJoystick() {
  int xVal = analogRead(JOYSTICK_PIN_X);
  int yVal = analogRead(JOYSTICK_PIN_Y);

  directionX = 0;
  directionY = 0;

  if (xVal >= 0 && xVal <= 512) {
    directionX = -1;
    directionY = 0;
  }
  if (xVal >= 3584 && xVal <= 4096) {
    directionX = 1;
    directionY = 0;
  }
  if (yVal >= 0 && yVal <= 512) {
    directionX = 0;
    directionY = -1;
  }
  if (yVal >= 3584 && yVal <= 4096) {
    directionX = 0;
    directionY = 1;
  }

  if (directionX == -1) {
    direction = MOVE_LEFT;
  }
  if (directionX == 1) {
    direction = MOVE_RIGHT;
  }
  if (directionY == -1) {
    direction = MOVE_UP;
  }
  if (directionY == 1) {
    direction = MOVE_DOWN;
  }
}

////////////////////////////////////////////////////
// 
//  ARDUINO STRUCTURE
//
////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Hello, ESP32-S3!");

  randomSeed(analogRead(0));

  snakeX = 6;
  snakeY = 3;
  setApple();

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(16);

  direction = MOVE_STOP;
  game.snake.init(snakeX, snakeY);
  FastLED.show();

  // asyncTimer.setInterval(loopTest, 1400);
  asyncTimer.setInterval(loopGame, 250);
  asyncTimer.setInterval(loopJoystick, 150);
}

void loop() {
  asyncTimer.handle();
}

void printDirection() {
  return;
  Serial.print("DIRECTION is: ");
  Serial.print(snakeX);
  Serial.print(" : ");
  Serial.println(snakeY);
}

void printPosition() {
  return;
  Serial.print("POSITION is: ");
  Serial.print(snakeX);
  Serial.print(" : ");
  Serial.println(snakeY);
}
