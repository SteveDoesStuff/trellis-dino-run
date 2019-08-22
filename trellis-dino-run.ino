#include "Adafruit_NeoTrellisM4.h"
#include "timer.h"

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

auto timer = timer_create_default();

#define rows 4
#define cols 8
int pixels[rows][cols] = {{0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {1,1,1,1,1,1,1,1}};

int next[4];
bool lastEmpty = true;

int dinoRow = 1;
bool jumping = false;
bool ducking = false;
bool standing = true;
bool hit = false;

int lives = 3;

int dinoColor = trellis.Color(255,255,0); // yellow

int scrollDelay = 500;

bool scroll(void *) {
  Serial.println("Scroll");

  if (!standing) {
    standing = true;
  }

  long type = random(0,100);
  if (lastEmpty == false || type < 20) {
    Serial.println("Empty");
    next[0] = 0;
    next[1] = 0;
    next[2] = 0;
    next[3] = 1;
    lastEmpty = true;
  } else if (type < 60) {
    Serial.println("Cactus");
    next[0] = 0;
    next[1] = 0;
    next[2] = 2; // cactus
    next[3] = 1;
    lastEmpty = false;
  } else {
    Serial.println("Pterodactyl");
    next[0] = 0;
    next[1] = 3; // pterodactyl
    next[2] = 0;
    next[3] = 1;
    lastEmpty = false;
  }

  int pixel = 0;

  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      if (col == 7) {
        pixels[row][col] = next[row];
      } else {
        pixels[row][col] = pixels[row][col + 1];
      }
      
      // draw the pixels
      if (col == 1 && row == dinoRow) {
        // draw the dino!
        trellis.setPixelColor(8 *  dinoRow + 1, trellis.Color(255,255,0));
      } else if (col == 1 && row == dinoRow + 1) {
        if (!ducking) {
          // draw the rest of the dino!
          trellis.setPixelColor(8 *  (dinoRow + 1) + 1, trellis.Color(255,255,0));
        }
      } else if (col == 5 && row == 0 && lives >= 3) {
        // draw the first heart
        trellis.setPixelColor(pixel, trellis.Color(255,0,0));
      } else if (col == 6 && row == 0 && lives >= 2) {
        // draw the second heart
        trellis.setPixelColor(pixel, trellis.Color(255,0,0));
      } else if (col == 7 && row == 0 && lives >= 1) {
        // draw the last heart
        trellis.setPixelColor(pixel, trellis.Color(255,0,0));
      } else if (col == 6 && row == 3) {
        // draw the A button
        trellis.setPixelColor(pixel, trellis.Color(0,0,255));
      } else if (col == 7 && row == 3) {
        // draw the B button
        trellis.setPixelColor(pixel, trellis.Color(0,0,255));
      } else {
        // draw the level
        if (pixels[row][col] == 0) {
          // empty
          trellis.setPixelColor(pixel, trellis.Color(0,0,0));
        } else if (pixels[row][col] == 1) {
          // ground
          trellis.setPixelColor(pixel, trellis.Color(255,255,255));
        } else if (pixels[row][col] == 2) {
          // cactus
          trellis.setPixelColor(pixel, trellis.Color(0,255,0));
        } else if (pixels[row][col] == 3) {
          // pterodactyl
          trellis.setPixelColor(pixel, trellis.Color(255,0,255));
        }
      }

      // did the dino hit something?
      if (pixel == 17 && pixels[row][col] == 2 && !jumping) {
        // it hit a cactus :(
        Serial.println("Hit cactus :(");
        hit = true;
      } else if (pixel == 9 && pixels[row][col] == 3 && !ducking) {
        // it hit a pterodactyl :(
        Serial.println("Hit pterodactyl :(");
        hit = true;
      }

      if (hit) {
        lives = lives - 1;
        hit = false;

        // remove a life right away
        int lifePixel = 7 - lives;
        trellis.setPixelColor(lifePixel, trellis.Color(0,0,0));
        
        if (lives == 0) {
          // game over!!!
//          timer.stop(scroll);
          return false;
        }
      }

      pixel++;
    }
  }

  if (jumping || ducking) {
    dinoRow = 1;
    jumping = false;
    ducking = false;
    standing = false;
  }

  return true; // !important - needed for the timer to work
}

void setup() {
  Serial.begin(115200);
  Serial.println("Init");

  randomSeed(analogRead(A0) * analogRead(A1) * analogRead(A2) * analogRead(A3));

  trellis.begin();
  trellis.setBrightness(80);

  timer.every(scrollDelay, scroll);
  Serial.println("Timer set");
}

void loop() {
  trellis.tick();
  timer.tick();
  
  while (trellis.available()) {
    keypadEvent e = trellis.read();

    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      int key = e.bit.KEY;
      if (!jumping && !ducking && standing) {
        if (key == 30) {
          Serial.println("Jump");
          jumping = true;
          dinoRow = 0;
          trellis.setPixelColor(8 *  dinoRow + 1, trellis.Color(255,255,0));
          trellis.setPixelColor(8 *  (dinoRow + 1) + 1, trellis.Color(255,255,0));
          trellis.setPixelColor(8 *  (dinoRow + 2) + 1, trellis.Color(0,0,0));
        } else if (key == 31) {
          Serial.println("Duck");
          ducking = true;
          dinoRow = 2;
          trellis.setPixelColor(8 *  dinoRow + 1, trellis.Color(255,255,0));
          trellis.setPixelColor(8 *  (dinoRow - 1) + 1, trellis.Color(0,0,0));
        }
      }
    }
  }
}
