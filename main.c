#include "raylib.h"
#include "stdio.h"
#include "unistd.h"
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
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
#define MYBEIGE 0
#define MYGRAY 1
#define MYGOLD 2

#define gencolorgrey(x)                                                        \
  CLITERAL(Color) { x, g2(x), b2(x), 255 } // Grey
#define gencolorgold(x)                                                        \
  CLITERAL(Color) { x, x - 40, 0, 255 } // Gold
#define gencolor(x)                                                            \
  CLITERAL(Color) { x, g(x), b(x), 255 } // Beige

#define BEIGE2                                                                 \
  CLITERAL(Color) { 230, 220, 210, 255 } // Beige

#define LINESHAPE 1
#define TSHAPE 0
#define LINESHAPE2 2
#define LINESHAPE3 3

const int hf = 12; // height ratio
const int wf = 10; // width ratio

bool wallMap[wallH + 1][wallW];
bool wallLine[wallH];
int goldenwall = 0;
int score = 0;
// inital position of object
// --- all these have to be reset  ON BRICK -----
float startPos_x = wallW / 2.0;
float startPos_y = 0;
size_t grid_depth = 0; // falling position
int orientation = 0;   // object orientation
int move = 0;          // lateral movement , relative from starting pos
int offset_x = 0;
bool reset_object = true;
// ------------------------------------

// function declarations
void InitWall();
void DrawMyGrid();
void DrawWall();
void DrawGoldenWall(int color);
void ResetWallLine();
void resetObject();
void DrawMiniBox(int x, int y);
void DrawWallMiniBox(int x, int y, int color);
void DrawObject(int x, int y, int direction);
void TBoxU(int startX, int StartY);
void TBoxD(int startX, int StartY);
void TBoxR(int startX, int StartY);
void TBoxL(int startX, int StartY);

typedef struct TShape {
  float x;
  float y;
  Vector2 a; //-
  Vector2 b; // -
  Vector2 c; //  -
  Vector2 d; // |
  int orientation;
  int new_orientation;
  bool brick;
  bool invalid_orientation;
} tshape_t;

typedef struct LineShape {
  float x;
  float y;
  int line_size;
  Vector2 a[4]; //-
  int orientation;
  int new_orientation;
  bool brick;
  bool invalid_orientation;
} lineshape_t;

void setBrick(int x, int y) { wallMap[y][x] = true; }

void RenderMiniBox(int x, int y, int color) {
  size_t depth = 3;
  size_t xp = x * box;
  size_t yp = y * box;
  Color color1, color2;
  switch (color) {
  case MYGRAY:
    color1 = gencolorgrey(200);
    color2 = gencolorgrey(180);
    break;
  case MYGOLD:
    color1 = gencolorgold(245);
    color2 = gencolorgold(255);
    break;
  case MYBEIGE:
    color1 = gencolor(220);
    color2 = gencolor(200);
    break;
  default:
    color1 = gencolorgold(color);
    color2 = gencolorgold(color + 10);
  }

  Rectangle rec = {xp, yp, box, box};
  DrawRectangleRec(rec, color1);
  Rectangle rec2 = {xp + depth, yp + depth, box - depth * 2, box - depth * 2};
  DrawRectangleRec(rec2, color2);
}

bool check_t_brick(tshape_t t) {
  if (wallMap[(int)t.a.y + 1][(int)t.a.x] == true) {
    // printf("BRICCKED1 ");
    return true;
  }
  if (wallMap[(int)t.b.y + 1][(int)t.b.x] == true) {
    // printf("BRICCKED2 ");
    return true;
  }
  if (wallMap[(int)t.c.y + 1][(int)t.c.x] == true) {
    // printf("BRICCKED3 ");
    return true;
  }
  if (wallMap[(int)t.d.y + 1][(int)t.d.x] == true) {
    // printf("BRICCKED4 ");
    return true;
  }
  return false;
}

bool check_line_brick(lineshape_t t) {
  for (int i = 0; i < t.line_size; i++) {
    if (wallMap[(int)t.a[i].y + 1][(int)t.a[i].x] == true) {
      // printf("BRICCKED1 ");
      return true;
    }
  }
  return false;
}
void render_all_line_box(lineshape_t *t1, int color) {
  for (int i = 0; i < t1->line_size; i++) {
    RenderMiniBox(t1->a[i].x, t1->a[i].y, color);
  }
}

void set_line_brick(lineshape_t *t) {
  for (int i = 0; i < t->line_size; i++) {
    setBrick(t->a[i].x, t->a[i].y);
  }
}

void set_t_brick(tshape_t *t) {
  setBrick(t->a.x, t->a.y);
  setBrick(t->b.x, t->b.y);
  setBrick(t->c.x, t->c.y);
  setBrick(t->d.x, t->d.y);
}

bool is_brick_wall(Vector2 v1) {
  if (wallMap[(int)v1.x][(int)v1.y] == true)
    return true;
  return false;
}

tshape_t design_tshape(tshape_t *t, bool skip_check) {
  tshape_t t2 = {};
  if (t->new_orientation == 0) {
    // draw up
    t2.a.x = t->x - 1;
    t2.a.y = t->y + 1;
    t2.b.x = t->x;
    t2.b.y = t->y + 1;
    t2.c.x = t->x + 1;
    t2.c.y = t->y + 1;
    t2.d.x = t->x;
    t2.d.y = t->y;
  } else if (t->new_orientation == 1) {
    // draw right
    t2.a.x = t->x;
    t2.a.y = t->y;
    t2.b.x = t->x;
    t2.b.y = t->y + 1;
    t2.c.x = t->x;
    t2.c.y = t->y + 2;
    t2.d.x = t->x + 1;
    t2.d.y = t->y + 1;
  } else if (t->new_orientation == 2) {
    // draw down
    t2.a.x = t->x - 1;
    t2.a.y = t->y;
    t2.b.x = t->x;
    t2.b.y = t->y;
    t2.c.x = t->x + 1;
    t2.c.y = t->y;
    t2.d.x = t->x;
    t2.d.y = t->y + 1;
  } else if (t->new_orientation == 3) {
    // draw left
    t2.a.x = t->x;
    t2.a.y = t->y;
    t2.b.x = t->x;
    t2.b.y = t->y + 1;
    t2.c.x = t->x;
    t2.c.y = t->y + 2;
    t2.d.x = t->x - 1;
    t2.d.y = t->y + 1;
    t2.brick = check_t_brick(t2);
  }
  if (!skip_check) {
    if (is_brick_wall(t2.a) || is_brick_wall(t2.b) || is_brick_wall(t2.c) ||
        is_brick_wall(t2.d)) {
      t2.invalid_orientation = true;
    }
    t2.brick = check_t_brick(t2);
  }
  t2.orientation = t2.new_orientation;
  return t2;
}

void render_all_t_box(tshape_t *t1, int color) {
  RenderMiniBox(t1->a.x, t1->a.y, color);
  RenderMiniBox(t1->b.x, t1->b.y, color);
  RenderMiniBox(t1->c.x, t1->c.y, color);
  RenderMiniBox(t1->d.x, t1->d.y, color);
}

void draw_t_shape(tshape_t *t1, bool skip_check) {
  tshape_t t2 = design_tshape(t1, skip_check);
  if (t2.invalid_orientation) {
    t1->new_orientation = t1->orientation;
    t2 = design_tshape(t1, skip_check);
  }
  t1 = &t2;
  // printf("brick status %d , t2 %d\n", t1->brick, t2.brick);
  if (t2.brick == true) {
    // printf("t2 setting bricks \n");
    set_t_brick(t1);
    resetObject();
    return;
  }
  render_all_t_box(t1, MYBEIGE);
}

lineshape_t design_line_shape(lineshape_t *t) {
  lineshape_t l2;
  l2 = *t;
  l2.line_size = t->line_size;
  if (t->new_orientation == 0) {
    // draw up
    for (int i = 0; i < l2.line_size; i++) {
      l2.a[i].x = t->x;
      l2.a[i].y = t->y + i + 1;
      if (is_brick_wall(l2.a[i])) {
        l2.invalid_orientation = true;
        return l2;
      }
    }
    l2.brick = check_line_brick(l2);
  } else if (t->new_orientation == 1) {
    // draw down
    for (int i = 0; i < l2.line_size; i++) {
      l2.a[i].x = t->x + i;
      l2.a[i].y = t->y;
      if (is_brick_wall(l2.a[i])) {
        l2.invalid_orientation = true;
        return l2;
      }
    }
    l2.brick = check_line_brick(l2);
  }
  l2.orientation = t->new_orientation;
  return l2;
}

void draw_line_shape(lineshape_t *l) {
  lineshape_t l1 = design_line_shape(l);
  if (l1.invalid_orientation) {
    l->new_orientation = l->orientation;
    l1 = design_line_shape(l);
  }
  l = &l1;
  // printf("brick status %d , t2 %d\n", t1->brick, t2.brick);
  if (l1.brick == true) {
    // printf("t2 setting bricks \n");
    set_line_brick(l);
    resetObject();
    return;
  }
  render_all_line_box(l, MYBEIGE);
}

int main(int argc, char *argv[]) {
  InitWindow(multiple * (wf + 10), multiple * (hf), "BRIKS");
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
  int golden_frame_counter = 0;
  // tshape_t *t1 = (tshape_t *)malloc(sizeof(tshape_t));
  tshape_t t1, tn;
  lineshape_t l1, ln;
  t1.brick = false;
  int pick_shape;
  int next_object = LINESHAPE;
  // printf("default brick status - %d ", t1.brick);
  // Main event loop for UI - check FPS for frequency
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // update the music stream buffer // this moves it ahead
    UpdateMusicStream(music);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawMyGrid();
    char buffer[50];
    sprintf(buffer, "Score - %d", score - 1);
    DrawText(buffer, (wf * multiple) + 10, 10, 40, DARKPURPLE);
    // printf("starting loop ");
    // Pause/Resume music playing with key p
    if (IsKeyPressed(KEY_P)) {
      pause = !pause;
      if (pause) {
        PauseMusicStream(music);
      } else
        ResumeMusicStream(music);
    }
    if (!pause) {
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
      // when reset pick a type of object
      if (reset_object) {
        srand(time(0));
        pick_shape = next_object;
        next_object = rand();
        next_object = next_object % 4;
        reset_object = false;
      }
    } else {
      DrawText("GAME PAUSED", wf * 5, hf * 5, 40, DARKGRAY);
    }
    // draw current object
    if (pick_shape == TSHAPE) {
      t1.new_orientation = orientation;
      t1.x = startPos_x + (float)move;
      t1.y = startPos_y + (float)grid_depth;
      draw_t_shape(&t1, false);
    } else if (pick_shape == LINESHAPE || pick_shape == LINESHAPE2 ||
               pick_shape == LINESHAPE3) {
      l1.new_orientation = orientation % 2;
      l1.x = startPos_x + (float)move;
      l1.y = startPos_y + (float)grid_depth;
      l1.line_size = pick_shape + 1;
      draw_line_shape(&l1);
    }
    // draw next object
    DrawText("Next object", (wf * multiple) + 10, hf * multiple / 2 - 50, 40,
             DARKGRAY);
    if (next_object == TSHAPE) {
      tn.new_orientation = 0;
      tn.x = wf * 2 + 5;
      tn.y = hf + 2;
      draw_t_shape(&tn, true);
    } else if (next_object == LINESHAPE || next_object == LINESHAPE2 ||
               next_object == LINESHAPE3) {
      ln.new_orientation = 0;
      ln.x = wf * 2 + 5;
      ln.y = hf;
      ln.line_size = next_object + 1;
      draw_line_shape(&ln);
    }
    // printf("Draw t1 x, y %f, %f \n", t1.x, t1.y);
    // FIXME : if not for this , it is segfaulting !!! something to do with
    // threads and asyc ??
    DrawWall();
    if (goldenwall > 0) {
      if (golden_frame_counter < MyFPS) {
        if (!pause)
          golden_frame_counter++;
        DrawGoldenWall(golden_frame_counter + 10);
      } else {
        golden_frame_counter = 0;
        ResetWallLine();
      }
    }
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
  for (int j = 0; j < wallW; j++) {
    wallMap[wallH][j] = true;
  }
}

void resetObject() {
  startPos_x = wallW / 2.0;
  startPos_y = 0;
  grid_depth = 0;  // falling position
  orientation = 0; // object orientation
  move = 0;        // lateral movement , relative from starting pos
  offset_x = 0;
  reset_object = true;
}

void DrawWall() {
  for (int i = 0; i <= wallH; i++) {
    int brickCounter = 0;
    for (int j = 0; j < wallW; j++) {
      if (wallMap[i][j] == true) {
        // printf("render box wall height, x- %d ,y- %d \n", j, i);
        RenderMiniBox(j, i, MYGRAY);
        brickCounter++;
      }
    }
    if (brickCounter == wallW) {
      if (!wallLine[i]) {
        wallLine[i] = true;
        goldenwall++;
        score++;
      }
    }
  }
}

void DrawGoldenWall(int color) {
  for (int i = 0; i < wallH; i++) {
    if (wallLine[i] == true) {
      for (int j = 0; j < wallW; j++) {
        if (wallMap[i][j] == true) {
          RenderMiniBox(j, i, 255 - color);
        }
      }
    }
  }
}

void ResetWallLine() {
  int i = wallH - 1;
  bool found = false;
  while (i > 0) {
    if (!found) {
      if (wallLine[i] == true)
        found = true;
      else {
        i--;
        continue;
      }
    }
    for (int j = 0; j < wallW; j++) {
      wallMap[i][j] = wallMap[i - 1][j];
    }
    wallLine[i] = wallLine[i - 1];
    i--;
  }
  goldenwall--;
  // printf("goldenwall - %d \n", goldenwall);
}

void DrawMyGrid() {
  for (int x = box; x < multiple * wf; x += box)
    DrawLine(x, 0, x, multiple * hf, BEIGE2);
  for (int y = box; y < multiple * hf; y += box)
    DrawLine(0, y, multiple * wf, y, BEIGE2);
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
    color2 = gencolorgold(255);
  }
  DrawRectangleRec(rec, color1);
  Rectangle rec2 = {xp + depth, yp + depth, box - depth * 2, box - depth * 2};
  DrawRectangleRec(rec2, color2);
}
