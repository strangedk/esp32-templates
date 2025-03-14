#include <Arduino.h>

const int cell = 3;
const int box = cell * cell;
const int total = box * box;

std::array<int,total> items = {0};

int indexFromXY(int x, int y) {
    return x * box + y;
}

struct StructPoint {
    int x;
    int y;
};

using Point = StructPoint;

Point xyFromIndex(int i) {
    Point p;
    p.x = i % box;
    p.y = i / box;
    return p;
}

std::array<int,box> row(int x) {
    std::array<int,box> result;
    for (int y = 0; y < box; ++y) {
        result[y] = items[indexFromXY(x, y)];
    }
    return result;
}

std::array<int,box> column(int y) {
    std::array<int,box> result;
    for (int x = 0; x < box; ++x) {
        result[x] = items[indexFromXY(x, y)];
    }
    return result;
}

std::array<int,box> getBox(int x, int y) {
    std::array<int,box> result;
    int startX = x * cell;
    int startY = y * cell;
    int idx = 0;
    for (int ix = startX; ix < startX + cell; ++ix) {
        for (int iy = startY; iy < startY + cell; ++iy) {
            result[idx++] = indexFromXY(ix, iy);
        }
    }
    return result;
}

bool isValid(int n, int x, int y) {
    for (int i = 0; i < box; ++i) {
        if (row(x)[i] == n || column(y)[i] == n) return false;
    }

    std::array<int,box> boxCells = getBox(x / cell, y / cell);
    for (int i = 0; i < box; ++i) {
        if (items[boxCells[i]] == n) return false;
    }
    return true;
}

bool solve() {
    for (int x = 0; x < box; ++x) {
        for (int y = 0; y < box; ++y) {
            if (items[indexFromXY(x, y)] == 0) {
                int nums[box];
                for (int i = 0; i < box; ++i) {
                    nums[i] = i + 1;
                }

                // Shuffle the numbers
                for (int i = 0; i < box; ++i) {
                    int j = random(box); // Random number between 0 and 8
                    int temp = nums[i];
                    nums[i] = nums[j];
                    nums[j] = temp;
                }

                for (int i = 0; i < box; ++i) {
                    int num = nums[i];
                    if (isValid(num, x, y)) {
                        items[indexFromXY(x, y)] = num;
                        if (solve()) {
                            return true;
                        }
                        items[indexFromXY(x, y)] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

void printGrid() {
    for (int i = 0; i < total; ++i) {
        if (i % box == 0) {
            Serial.println();
        }
        Serial.print(items[i]);
        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(9600);
    randomSeed(analogRead(0)); // Initialize random number generator

    delay(2000);

    Serial.println("Algo is starting...");

    if (solve()) {
        printGrid();
    } else {
        Serial.println("No solution found");
    }

    Serial.println("Solution is ready.");
}

void loop() {
    // Empty loop since we don't need continuous processing
}
