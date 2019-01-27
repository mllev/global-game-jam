#include <stdio.h>
#include <time.h>
#include <string.h>

#include "font.h"

#define WINDOW_IMPLEMENTATION
#include "window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define M8_IMPLEMENTATION
#include "mode8.h"

#define MAX_ROOMS 100
#define MAX_TILES 1000

typedef struct vec2 Vec2;
typedef struct vec2i Vec2i;
typedef struct player Player;
typedef struct room Room;
typedef struct game Game;

struct vec2 { float x, y; };
struct vec2i { int x, y; };

struct player {
  float x, y;
  Vec2 forward;
  float speed;
  int texture_start_index;
  int texture_end_index;
  float angle;
  int is_accelerating;
};

struct room {
  int x1, y1;
  int x2, y2;
  int center_x, center_y;
};

struct game {
  Room rooms[MAX_ROOMS];
  Vec2i tiles[MAX_TILES];
  int room_count;
  int tile_count;
  int floor_texture_index;
  int screen_height;
  int screen_width;
  struct {
    float x, y;
  } camera;
};

Game GAME;

void init_player (Player *p)
{
  p->x = 0;
  p->y = 0;
  p->forward.x = 0;
  p->forward.y = -1;
  p->speed = 0.0;
  p->angle = 0.0;
  p->is_accelerating = 0;
  p->texture_start_index = 0;
  p->texture_end_index = 0;
}

void update_player (Player* p)
{
  if (p->is_accelerating) {
    p->speed += 0.1;
  } else {
    p->speed -= 0.1;
  }

  if (p->speed > 4.0) {
    p->speed = 4.0;
  }

  if (p->speed < 0.0) {
    p->speed = 0.0;
  }

  if (p->speed > 0) {
    p->x += (p->forward.x * p->speed);
    p->y += (p->forward.y * p->speed);
  }
}

void rotate_player (Player *p, float a)
{
  float c = cosf(a);
  float s = sinf(a);

  float x = c * p->forward.x - s * p->forward.y;
  float y = c * p->forward.y + s * p->forward.x;

  p->forward.x = x;
  p->forward.y = y;
  p->angle += a;

  if (p->angle > (M8_PI * 2)) {
    p->angle = 0;
  } else if (p->angle < 0) {
    p->angle += (M8_PI * 2);
  }
}

Vec2 project_to_screen (float x, float y)
{
  Vec2 v;
  v.x = (x - GAME.camera.x) + ((float)GAME.screen_width / 2.0);
  v.y = (y - GAME.camera.y) + ((float)GAME.screen_height / 2.0);
  return v;
}

void draw_player (Player* p)
{
  int max = (p->texture_end_index - p->texture_start_index) + 1;
  int index = (int)floor((p->angle / (M8_PI * 2)) * (float)max);
  Vec2 v = project_to_screen(p->x, p->y);

  m8_draw_tile_scaled(v.x, v.y, 64, 64, 32, index + p->texture_start_index);
}

void draw_world ()
{
  int i;
  int tilesize = 64;

  for (i = 0; i < GAME.tile_count; i++) {
    int tx = GAME.tiles[i].x * tilesize;
    int ty = GAME.tiles[i].y * tilesize;

    Vec2 v = project_to_screen((float)tx, (float)ty);
    m8_draw_tile_scaled(v.x, v.y, tilesize, tilesize, 32, GAME.floor_texture_index);
  }
}

unsigned int rand_between(unsigned int min, unsigned int max)
{
  return (rand() % (max + 1 - min)) + min;
}

int room_intersects_rooms (Room *r1)
{
  Room *r2;
  int i, max = GAME.room_count;
  int r1x1, r1x2, r1y1, r1y2;
  int r2x1, r2x2, r2y1, r2y2;

  /* for padding */
  r1x1 = r1->x1 - 1;
  r1y1 = r1->y1 - 1;
  r1x2 = r1->x2 + 1;
  r1y2 = r1->y2 + 1;

  for (i = 0; i < max; i++) {
    r2 = &GAME.rooms[i];

    r2x1 = r2->x1;
    r2y1 = r2->y1;
    r2x2 = r2->x2;
    r2y2 = r2->y2;

    if (r1x1 <= r2x2 && r1x2 >= r2x1 && r1y1 <= r2y2 && r2y2 >= r2y1) {
      return 1;
    }
  }

  return 0;
}

Room generate_room ()
{
  Room r;
  int x, y, w, h;

  x = rand_between(0, 15);
  y = rand_between(0, 15);

  w = rand_between(1, 5);
  h = rand_between(1, 5);

  r.x1 = x;
  r.y1 = y;

  r.x2 = x + w;
  r.y2 = y + h;

  r.center_x = x + (w / 2);
  r.center_y = y + (w / 2);

  return r;
}

void add_tile (int x, int y)
{
  int i, idx;
  if (GAME.tile_count >= MAX_TILES) return;
  for (i = 0; i < GAME.tile_count; i++) {
    if (GAME.tiles[i].x == x && GAME.tiles[i].y == y) {
      return;
    }
  }
  idx = GAME.tile_count++;
  GAME.tiles[idx].x = x;
  GAME.tiles[idx].y = y;
}

void add_room (Room *r)
{
  int x, y;

  for (y = r->y1; y <= r->y2; y++) {
    for (x = r->x1; x <= r->x2; x++) {
      add_tile(x, y);
    }
  }

  GAME.rooms[GAME.room_count++] = *r;
}

void add_vertical_hall (int x, int y, int ymax)
{
  if (ymax > y) {
    while (y <= ymax) {
      add_tile(x, y++);
    }
  } else {
    while (ymax <= y) {
      add_tile(x, ymax++);
    }
  }
}

void add_horizontal_hall (int x, int y, int xmax)
{
  if (xmax > x) {
    while (x <= xmax) {
      add_tile(x++, y);
    }
  } else {
    while (xmax <= x) {
      add_tile(xmax++, y);
    }
  }
}

void add_hall (Room *r1, Room *r2)
{
  add_vertical_hall(r1->center_x, r1->center_y, r2->center_y);
  add_horizontal_hall(r2->center_x, r2->center_y, r1->center_x);
}

void generate_world ()
{
  int i, max_tries = 10000000;
  Room r;

  r.x1 = 0;
  r.y1 = 0;
  r.x2 = rand_between(1, 4);
  r.y2 = rand_between(1, 4);
  r.center_x = r.x2 / 2;
  r.center_y = r.y2 / 2;

  add_room(&r);

  for (i = 0; i < max_tries; i++) {
    if (GAME.room_count >= MAX_ROOMS) return;
    r = generate_room();
    if (!room_intersects_rooms(&r)) {
      add_room(&r);
      add_hall(&r, &GAME.rooms[GAME.room_count - 2]);
    }
  }
}

void init_game (int width, int height)
{
  GAME.room_count = 0;
  GAME.tile_count = 0;
  GAME.screen_width = width;
  GAME.screen_height = height;

  generate_world();
}

void run (window_t *window, m8u32 *framebuffer, int width, int height)
{
  char debug_string[50];
  unsigned int frame, start;
  int tw, th, channels;
  unsigned char *bmp;
  Mode8 *ctx;
  int start_index, end_index;
  Player robo;

  ctx = malloc(sizeof(Mode8));

  m8_use_context(ctx);
  m8_init(framebuffer, width, height);

  init_game(width, height);
  init_player(&robo);

  bmp = stbi_load("robovac-sheet.png", &tw, &th, &channels, 0);
  m8_import_tiles_from_image(bmp, tw, th, channels, 32, &start_index, &end_index);
  robo.texture_start_index = start_index;
  robo.texture_end_index = end_index;

  free(bmp);

  bmp = stbi_load("floortile.png", &tw, &th, &channels, 0);
  m8_import_tiles_from_image(bmp, tw, th, channels, 32, &start_index, &end_index);
  GAME.floor_texture_index = start_index;

  free(bmp);

  while (!window->quit) {
    start = SDL_GetTicks();

    if (window->keys.w) {
      robo.is_accelerating = 1;
    } else {
      robo.is_accelerating = 0;
    }

    if (robo.speed <= 0) {
      if (window->keys.a) {
        rotate_player(&robo, 0.15);
      } else if (window->keys.d) {
        rotate_player(&robo, -0.15);
      }
    }

    update_player(&robo);

    GAME.camera.x = robo.x + 32;
    GAME.camera.y = robo.y + 32;

    draw_world();
    draw_player(&robo);

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "frame: %dms", frame);
    m8_draw_text_8x8(ascii, debug_string, (int)strlen(debug_string), 0, 0);
    window_update(window, framebuffer);
    m8_clear();
  }

  window_close(window);
  free(framebuffer);
}

int main (void) {
  int width = 960, height = 540;
  m8u32* framebuffer;
  window_t window;

  srand(time(NULL));

  if (!(framebuffer = (m8u32 *)malloc((width * height) * sizeof(m8u32)))) {
    return 1;
  }

  window_open(&window, "game", width, height, 1280, 720);
  run(&window, framebuffer, width, height);
}
