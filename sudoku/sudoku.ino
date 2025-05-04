#include <Arduino.h>
#include <TFT_eSPI.h>

typedef struct Point {
    int x;
    int y;
} Point;

const int cell = 3;
const int box = cell * cell;
const int total = box * box;

int items[total] = {0}; // Sudoku grid
int rowResult[box];
int columnResult[box];
int boxResult[box];

TFT_eSPI tft = TFT_eSPI();

// Coordinate helpers
int indexFromXY(int x, int y) {
    return x * box + y;
}

Point xyFromIndex(int i) {
    Point p;
    p.x = i % box;
    p.y = i / box;
    return p;
}

// Row / column / box helpers
int* row(int x) {
    for (int y = 0; y < box; ++y) {
        rowResult[y] = items[indexFromXY(x, y)];
    }
    return rowResult;
}

int* column(int y) {
    for (int x = 0; x < box; ++x) {
        columnResult[x] = items[indexFromXY(x, y)];
    }
    return columnResult;
}

int* getBox(int x, int y) {
    int startX = x * cell;
    int startY = y * cell;
    int idx = 0;

    for (int ix = startX; ix < startX + cell; ++ix) {
        for (int iy = startY; iy < startY + cell; ++iy) {
            boxResult[idx++] = items[indexFromXY(ix, iy)];
        }
    }
    return boxResult;
}

bool isValid(int n, int x, int y) {
    for (int i = 0; i < box; ++i) {
        if (row(x)[i] == n || column(y)[i] == n) return false;
    }

    int* boxCells = getBox(x / cell, y / cell);
    for (int i = 0; i < box; ++i) {
        if (boxCells[i] == n) return false;
    }
    return true;
}

bool solve() {
    for (int x = 0; x < box; ++x) {
        for (int y = 0; y < box; ++y) {
            if (items[indexFromXY(x, y)] == 0) {
                int nums[box];
                for (int i = 0; i < box; ++i) nums[i] = i + 1;

                for (int i = 0; i < box; ++i) {
                    int j = random(box);
                    int temp = nums[i];
                    nums[i] = nums[j];
                    nums[j] = temp;
                }

                for (int i = 0; i < box; ++i) {
                    int num = nums[i];
                    if (isValid(num, x, y)) {
                        items[indexFromXY(x, y)] = num;
                        if (solve()) return true;
                        items[indexFromXY(x, y)] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

// Draw Sudoku grid on TFT
void drawGrid() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);

    int screenW = tft.width();    // 320
    int screenH = tft.height();   // 480
    int gridSize = min(screenW, screenH) - 20;
    int cellSize = gridSize / box;
    int xOffset = (screenW - gridSize) / 2;
    int yOffset = (screenH - gridSize) / 2;

    // Draw lines
    for (int i = 0; i <= box; i++) {
        int thickness = (i % cell == 0) ? 2 : 1;
        tft.drawFastHLine(xOffset, yOffset + i * cellSize, gridSize, TFT_WHITE);
        tft.drawFastVLine(xOffset + i * cellSize, yOffset, gridSize, TFT_WHITE);
    }

    // Draw numbers
    for (int x = 0; x < box; x++) {
        for (int y = 0; y < box; y++) {
            int val = items[indexFromXY(x, y)];
            if (val != 0) {
                int cx = xOffset + y * cellSize + cellSize / 2;
                int cy = yOffset + x * cellSize + cellSize / 2;
                tft.drawNumber(val, cx, cy);
            }
        }
    }
}

void setup() {
    tft.init();
    tft.setRotation(1); // Adjust depending on orientation
    tft.fillScreen(TFT_BLACK);
    delay(500); // Just in case

    randomSeed(analogRead(0) + micros()); // Initialize RNG

    bool success = solve();
    if (success) {
        drawGrid();
    } else {
        tft.setTextDatum(TC_DATUM);
        tft.setTextSize(3);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("No solution", tft.width() / 2, tft.height() / 2);
    }
}

void loop() {
    // Nothing here (static display)
}
