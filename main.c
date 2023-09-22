#include "raylib.h"
#include "stdio.h"
#include "unistd.h"
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <threads.h>

#define g(x) x - 10
#define b(x) x - 20

#define g2(x) x + 5
#define b2(x) x + 10

#define g3(x) x - 50

#define MyFPS 60
#define multiple 50
#define box 25 // half the multiple
#define fallspeed(x) x / 2
#define wallH 24 // hf * multiple / box
#define wallW 20 // wf * multiple / box

// colors
#define MYGRAY 1
#define MYGOLD 2

#define gencolorgrey(x)                                                        \
  CLITERAL(Color) { x, g2(x), b2(x), 255 } // Beige

#define gencolorgold(x)                                                        \
  CLITERAL(Color) { x, x - 50, 50, 255 } // Gold
#define gencolor(x)                                                            \
  CLITERAL(Color) { x, g(x), b(x), 255 } // Beige

#define BEIGE2                                                                 \
  CLITERAL(Color) { 230, 220, 210, 255 } // Beige

// function declarations
void InitWall();
void DrawMyGrid();
void DrawWall();
void DrawGoldenWall();
void ResetWallLine();
void DrawMiniBox(int x, int y);
void DrawWallMiniBox(int x, int y, int color);
void DrawObject(int x, int y, int direction);
void TBoxU(int startX, int StartY);
void TBoxD(int startX, int StartY);
void TBoxR(int startX, int StartY);
void TBoxL(int startX, int StartY);

const int hf = 12; // height ratio
const int wf = 10; // width ratio

bool wallMap[wallH + 1][wallW];
bool wallLine[wallH];

// inital position of object
// --- all these have to be reset  ON BRICK -----
size_t startPos_x = wallW / 2;
size_t startPos_y = 0;
size_t grid_depth = 0; // falling position
int orientation = 0;   // object orientation
int move = 0;          // lateral movement , relative from starting pos
int offset_x = 0;
bool make_object_brick = false;
// ------------------------------------

bool reset_object = false;
int goldenwall = 0;

int main(int argc, char *argv[]) {

  InitWindow(multiple * wf, multiple * hf, "Hello World");
  InitAudioDevice();
  Music music = LoadMusicStream("resources/t-2.mp3");
  // Music music = LoadMusicStream("resources/country.mp3");
  printf("music info total frame count in the music object : %u\n",
         music.frameCount);
  printf("music info context type int: %d\n", music.ctxType);
  printf("music info sample rate int: %d\n", music.stream.sampleRate);
  printf("music info samplesize bit: %d\n", music.stream.sampleSize);
  PlayMusicStream(music);
  SetTargetFPS(MyFPS); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------
  int frame_counter = 0;
  SetConfigFlags(
      FLAG_MSAA_4X_HINT |
      FLAG_WINDOW_RESIZABLE); // Set MSAA 4X hint before windows creation
  bool pause = false;
  // Main game loop
  InitWall();

  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // update the music stream buffer // this moves it ahead
    UpdateMusicStream(music);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawMyGrid();
    frame_counter++;
    if (frame_counter > fallspeed(MyFPS)) {
      frame_counter = 0;
      grid_depth += 1;
    }
    if (IsKeyPressed(KEY_S))
      orientation = (orientation + 1) % 4;
    if (IsKeyPressed(KEY_RIGHT)) {
      move += 1;
    }
    if (IsKeyPressed(KEY_LEFT)) {
      move -= 1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
      grid_depth++;
    }
    DrawObject(startPos_x + move, startPos_y + grid_depth, orientation);
    DrawWall();
    DrawGoldenWall();
    if (goldenwall > 0) ResetWallLine();

    // DrawFPS(10, 10);
    EndDrawing();
  }

  // De-Initialization
  //---------------------------------------------------------
  UnloadMusicStream(music); // Unload music stream buffers from RAM

  CloseAudioDevice(); // Close audio device (music streaming is automatically
                      // stopped)
  CloseWindow();      // Close window and OpenGL context
  //----------------------------------------------------------

  return 0;
}

void InitWall() {
  for (int i = 0; i <= wallH; i++) {
    for (int j = 0; j < wallW; j++) {
      if (i == wallH) {
        wallMap[i][j] = true;
      } else
        wallMap[i][j] = false;
    }
  }
}

void resetObject() {
  startPos_x = wallW / 2;
  startPos_y = 0;
  grid_depth = 0;  // falling position
  orientation = 0; // object orientation
  move = 0;        // lateral movement , relative from starting pos
  offset_x = 0;
  make_object_brick = false;
}

void DrawWall() {
  for (int i = 0; i < wallH; i++) {
    int brickCounter = 0;
    for (int j = 0; j < wallW; j++) {
      if (wallMap[i][j] == true) {
        DrawWallMiniBox(j, i, MYGRAY);
        brickCounter++;
      }
    }
    if (brickCounter == wallW) {
      wallLine[i] = true;
      goldenwall++;
    }
  }
}

void DrawGoldenWall() {
  for (int i = 0; i < wallH; i++) {
    if (wallLine[i] == true) {
      for (int j = 0; j < wallW; j++) {
        if (wallMap[i][j] == true) {
          DrawWallMiniBox(j, i, MYGOLD);
        }
      }
    }
  }
}

void ResetWallLine() {
  int i = wallH - 1;
  while (i > 0) {
    for (int j = 0; j < wallW; j++) {
      wallMap[i][j] = wallMap[i - 1][j];
    }
    wallLine[i] = wallLine[i - 1];
    i--;
  }
  goldenwall--;
}

bool checkWall(int x, int y) {
  if (wallMap[y + 1][x] == true) {
    make_object_brick = true;
    return true;
  }
  return false;
}

void setBrick(int x, int y) { wallMap[y][x] = true; }
void DrawMyGrid() {
  for (int x = box; x < multiple * wf; x += box)
    DrawLine(x, 0, x, multiple * hf, BEIGE2);
  for (int y = box; y < multiple * hf; y += box)
    DrawLine(0, y, multiple * wf, y, BEIGE2);
}

void DrawMiniBox(int x, int y) {
  size_t depth = 3;
  size_t xp = x * box;
  size_t yp = y * box;
  Rectangle rec = {xp, yp, box, box};
  DrawRectangleRec(rec, gencolor(220));
  Rectangle rec2 = {xp + depth, yp + depth, box - depth * 2, box - depth * 2};
  DrawRectangleRec(rec2, gencolor(200));
  if (checkWall(x, y) || make_object_brick) {
    setBrick(x, y);
    reset_object = true;
  }
}

void DrawWallMiniBox(int x, int y, int color) {
  size_t depth = 3;
  size_t xp = x * box;
  size_t yp = y * box;
  Rectangle rec = {xp, yp, box, box};
  Color color1, color2;
  if (color == MYGRAY) {
    color1 = gencolorgrey(200);
    color2 = gencolorgrey(180);
  }
  if (color == MYGOLD) {
    color1 = gencolorgold(245);
    color2 = gencolorgold(235);
  }
  DrawRectangleRec(rec, color1);
  Rectangle rec2 = {xp + depth, yp + depth, box - depth * 2, box - depth * 2};
  DrawRectangleRec(rec2, color2);
}

void DrawObject(int x, int y, int direction) {
  if (direction == 0) {
    // draw up
    TBoxU(x, y);
  } else if (direction == 1) {
    // draw down
    TBoxD(x, y);
  } else if (direction == 2) {
    // draw down
    TBoxR(x, y);
  } else if (direction == 3) {
    // draw down
    TBoxL(x, y);
  }
  // if object bricked, reset the object
  if (reset_object == true) {
    // check if line is any complete
    resetObject();
    reset_object = false;
  }
}

void TBoxD(int startX, int startY) {
  DrawMiniBox(startX + 1, startY + 2);
  for (int i = startX; i < startX + 3; i++) {
    DrawMiniBox(i, startY + 1);
  }
}

void TBoxU(int startX, int startY) {
  for (int i = startX; i < startX + 3; i++) {
    DrawMiniBox(i, startY + 1);
  }
  DrawMiniBox(startX + 1, startY);
}

void TBoxR(int startX, int startY) {
  for (int i = startY + 3; i > startY; i--) {
    DrawMiniBox(startX + 1, i);
  }
  DrawMiniBox(startX + 2, startY + 2);
}

void TBoxL(int startX, int startY) {
  for (int i = startY + 3; i > startY; i--) {
    DrawMiniBox(startX + 1, i);
  }
  DrawMiniBox(startX, startY + 2);
}
