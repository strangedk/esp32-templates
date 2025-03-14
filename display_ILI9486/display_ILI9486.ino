#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

int w = 480;
int h = 320;

#define NUM_STARS 120
#define STAR_LAYERS 7

struct Star {
  float x, y;
  float speed;
  uint16_t color;
};

Star stars[NUM_STARS];

float shipX = w / 2;
float shipY = h - 40;
float shipMoveX = 1;
float shipMoveY = 0;

void initStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = random(w);
    stars[i].y = random(h);
    stars[i].speed = 0.1 + (float)(i % STAR_LAYERS); 
    stars[i].color = TFT_BLACK;tft.color565(240 + random(16), 240 + random(16), 240 + random(16));
  }
}

void drawStarship() {
  tft.fillTriangle(shipX, shipY, shipX - 10, shipY + 20, shipX + 10, shipY + 20, TFT_WHITE);
  tft.fillRect(shipX - 3, shipY + 20, 6, 10, TFT_WHITE);

  shipY += shipMoveY;
  shipX += shipMoveX;

  if (shipX > 300)
    shipMoveX *= -1;
  if (shipX < 180)
    shipMoveX *= -1;

  tft.fillTriangle(shipX, shipY, shipX - 10, shipY + 20, shipX + 10, shipY + 20, TFT_BLUE);
  tft.fillRect(shipX - 3, shipY + 20, 6, 10, TFT_GREEN);
}

void updateStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    tft.drawPixel(stars[i].x, stars[i].y, TFT_WHITE);

    stars[i].y += stars[i].speed;

    if (stars[i].y >= h) {
      stars[i].y = 0;
      stars[i].x = random(w);
      stars[i].speed = 0.5 + (float)(i % STAR_LAYERS);
    }

    tft.drawPixel(stars[i].x, stars[i].y, stars[i].color);
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  initStars();
}

void loop() {
  updateStars();

  drawStarship();

  delay(20);
}
