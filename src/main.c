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
#define MAX_DIRT 4000

#define CLAMP(x,a,b) ((x)>(a)?((x)<(b)?(x):(b)):(a))

typedef struct vec2 Vec2;
typedef struct vec2i Vec2i;
typedef struct player Player;
typedef struct room Room;
typedef struct game Game;
typedef struct tile Tile;
typedef struct dirt Dirt;
typedef struct cat Cat;

struct vec2 { float x, y; };
struct vec2i { int x, y; };

struct dirt {
  int x, y;
  int texture_index;
  int collected;
};

struct tile {
  int x, y;
  unsigned int flags;
};

struct cat {
  float x, y;
  int texture_index;
  int tile_size;
  int angle_index;
};

struct player {
  float x, y;
  Vec2 forward;
  float speed;
  int texture_start_index;
  int texture_end_index;
  float angle;
  int is_accelerating;
  int is_reversing;
  int can_accelerate;
  int tile_size;
  int angle_index;
  int score;
  float charge;
};

struct room {
  int x1, y1;
  int x2, y2;
  int center_x, center_y;
};

struct game {
  Room rooms[MAX_ROOMS];
  Tile tiles[MAX_TILES];
  Dirt dirt[MAX_DIRT];
  int tile_size;
  int dirt_count;
  int room_count;
  int tile_count;
  int dust_texture_index;
  int floor_texture_index;
  int cat_walk_start_index;
  int cat_walk_end_index;
  int wall_texture_index;
  int screen_height;
  int screen_width;
  struct {
    float x, y;
  } camera;
};

Game GAME;

Vec2 angles[] = {
  { 0.0,  -1.0 },
  { 1.0,  -1.0 },
  { 1.0,   0.0 },
  { 1.0,   1.0 },
  { 0.0,   1.0 },
  { -1.0,  1.0 },
  { -1.0,  0.0 },
  { -1.0, -1.0 }
};

Vec2 project_to_screen (float x, float y)
{
  Vec2 v;
  v.x = (x - GAME.camera.x) + ((float)GAME.screen_width / 2.0);
  v.y = (y - GAME.camera.y) + ((float)GAME.screen_height / 2.0);
  return v;
}

unsigned int rand_between(unsigned int min, unsigned int max)
{
  return (rand() % (max + 1 - min)) + min;
}

void init_cat (Cat *c)
{
  c->x = 10.0;
  c->y = 10.0;
  c->angle_index = 0;
  c->texture_index = 0;
  c->tile_size = 64;
}

void draw_cat (Cat *c)
{
  Vec2 v = project_to_screen(c->x, c->y);
  m8_draw_tile_scaled(v.x, v.y, c->tile_size, c->tile_size, 32, GAME.cat_walk_start_index + c->texture_index);
}

int cat_has_collided (Cat *p)
{
  float player_width = (float)p->tile_size;
  float pad = player_width / 4.0;
  float tile_width = (float)GAME.tile_size;
  Vec2i tiles[9];
  int tidx = 0;
  int min_y, max_y, min_x, max_x;
  int x, y, i, j;
  unsigned int flags = 0;

  min_x = (int)floor((p->x + pad) / tile_width);
  min_y = (int)floor((p->y + pad) / tile_width);
  max_x = (int)ceil(((p->x + player_width - pad)) / tile_width);
  max_y = (int)ceil(((p->y + player_width - pad)) / tile_width);

  for (y = min_y; y < max_y; y++) {
    for (x = min_x; x < max_x; x++) {
      flags |= (1 << tidx);
      tiles[tidx].x = x;
      tiles[tidx++].y = y;
    }
  }

  for (i = 0; i < GAME.tile_count; i++) {
    for (j = 0; j < tidx; j++) {
      if (GAME.tiles[i].x == tiles[j].x && GAME.tiles[i].y == tiles[j].y) {
        flags &= ~(1 << j);
      }
    }
  }

  return flags > 0;
}

void animate_cat (Cat *c, int min, int max)
{
  static float tick = 1.0;
  if (c->texture_index < min || c->texture_index > max) {
    c->texture_index = min;
  }
  if (tick <= 0) {
    c->texture_index = c->texture_index + 1;
    if (c->texture_index > max) {
      c->texture_index = min;
    }
    tick = 1.0;
  } else {
    tick -= 0.15;
  }
}

void update_cat (Cat *c)
{
  float xspeed = angles[c->angle_index].x * 0.5;
  float yspeed = angles[c->angle_index].y * 0.5;

  c->x += xspeed;
  c->y += yspeed;

  if (cat_has_collided(c)) {
    c->x -= xspeed;
    c->y -= yspeed;

    c->angle_index = rand_between(0, 7);
  }

  animate_cat(c, 0, 3);
}

void init_player (Player *p)
{
  p->x = 5.0;
  p->y = 5.0;
  p->forward.x = 0.0;
  p->forward.y = -1.0;
  p->speed = 0.0;
  p->angle = 0.0;
  p->score = 0;
  p->charge = 5.0;

  p->tile_size = 32;

  p->angle_index = 0;
  p->can_accelerate = 1;
  p->is_reversing = 0;
  p->is_accelerating = 0;
}

int has_collided (Player *p)
{
  float player_width = (float)p->tile_size;
  float pad = player_width / 4.0;
  float tile_width = (float)GAME.tile_size;
  Vec2i tiles[9];
  int tidx = 0;
  int min_y, max_y, min_x, max_x;
  int x, y, i, j;
  unsigned int flags = 0;

  min_x = (int)floor((p->x + pad) / tile_width);
  min_y = (int)floor((p->y + pad) / tile_width);
  max_x = (int)ceil(((p->x + player_width - pad)) / tile_width);
  max_y = (int)ceil(((p->y + player_width - pad)) / tile_width);

  for (y = min_y; y < max_y; y++) {
    for (x = min_x; x < max_x; x++) {
      flags |= (1 << tidx);
      tiles[tidx].x = x;
      tiles[tidx++].y = y;
    }
  }

  for (i = 0; i < GAME.tile_count; i++) {
    for (j = 0; j < tidx; j++) {
      if (GAME.tiles[i].x == tiles[j].x && GAME.tiles[i].y == tiles[j].y) {
        flags &= ~(1 << j);
      }
    }
  }

  return flags > 0;
}

void update_player (Player* p)
{
  if (p->is_reversing) {
    if (p->speed < 0) {
      p->x += (p->forward.x * p->speed);
      p->y += (p->forward.y * p->speed);

      if (!has_collided(p)) {
        p->speed += 0.2;
      }
    } else {
      p->speed = 0;
      p->is_reversing = 0;
    }

    return;
  }

  if (p->is_accelerating) {
    if (p->can_accelerate) {
      p->speed += 0.2;
    }
  } else {
    p->can_accelerate = 1;
    p->speed -= 0.2;
  }

  if (p->speed > 7.0) {
    p->speed = 7.0;
  }

  if (p->speed < 0.0) {
    p->speed = 0.0;
    p->is_reversing = 0;
  }

  if (p->speed > 0) {
    p->x += (p->forward.x * p->speed);
    p->y += (p->forward.y * p->speed);
  }

  if (has_collided(p)) {
    p->speed = 0;
    p->is_reversing = 1;
    p->can_accelerate = 0;
    p->speed = -2.0;
  }

  p->charge -= 0.001;
}

void clean_dirt (Player *p)
{
  float half = p->tile_size / 2.0;
  float cx = p->x + half;
  float cy = p->y + half;

  int i, max = GAME.dirt_count;
  for (i = 0; i < max; i++) {
    if (!GAME.dirt[i].collected) {
      float dx = (float)GAME.dirt[i].x + 16.0;
      float dy = (float)GAME.dirt[i].y + 16.0;
      if (fabs(dx - cx) < 10 && fabs(dy - cy) < 10) {
        GAME.dirt[i].collected = 1;
        p->score++;
      }
    }
  }
}

void rotate_player (Player *p, float a)
{
  p->angle += a;

  if (p->angle < -1) {
    p->angle = 0;
    if (p->angle_index == 0) p->angle_index = 7;
    else p->angle_index--;
    p->forward.x = angles[p->angle_index].x;
    p->forward.y = angles[p->angle_index].y;
  } else if (p->angle > 1) {
    p->angle = 0;
    if (p->angle_index == 7) p->angle_index = 0;
    else p->angle_index++;
    p->forward.x = angles[p->angle_index].x;
    p->forward.y = angles[p->angle_index].y;
  }
}

void draw_player (Player* p)
{
  Vec2 v = project_to_screen(p->x, p->y);

  m8_draw_tile_scaled(v.x, v.y, p->tile_size, p->tile_size, 32, p->texture_start_index + p->angle_index);
}

void draw_world ()
{
  int i;
  int tile_size = GAME.tile_size;

  for (i = 0; i < GAME.tile_count; i++) {
    int tx = GAME.tiles[i].x * tile_size;
    int ty = GAME.tiles[i].y * tile_size;

    Vec2 v = project_to_screen((float)tx, (float)ty);
    m8_draw_tile_scaled(v.x, v.y, tile_size, tile_size, 32, GAME.floor_texture_index);

    if (GAME.tiles[i].flags & (1 << 1)) {
      m8_draw_tile_scaled(v.x, v.y + (float)tile_size, tile_size, tile_size, 32, GAME.wall_texture_index);
    }
  }
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

void draw_dirt ()
{
  int i, max = GAME.dirt_count;
  for (i = 0; i < max; i++) {
    if (GAME.dirt[i].collected) continue;
    Vec2 v = project_to_screen((float)GAME.dirt[i].x, (float)GAME.dirt[i].y);
    m8_draw_tile_scaled(v.x, v.y, 32, 32, 32, GAME.dirt[i].texture_index);
  }
}

void add_dirt_pile (int x, int y, int id)
{
  int num = rand();

  if (num & 1) {
    if (num & 2) {
      Dirt *d = &GAME.dirt[GAME.dirt_count++];
      d->x = x * GAME.tile_size;
      d->y = y * GAME.tile_size;
      d->texture_index = id;
      d->collected = 0;
    }
    if (num & 4) {
      Dirt *d = &GAME.dirt[GAME.dirt_count++];
      d->x = x * GAME.tile_size + 32;
      d->y = y * GAME.tile_size;
      d->texture_index = id;
      d->collected = 0;
    }
    if (num & 8) {
      Dirt *d = &GAME.dirt[GAME.dirt_count++];
      d->x = x * GAME.tile_size;
      d->y = y * GAME.tile_size + 32;
      d->texture_index = id;
      d->collected = 0;
    }
    if (num & 16) {
      Dirt *d = &GAME.dirt[GAME.dirt_count++];
      d->x = x * GAME.tile_size + 32;
      d->y = y * GAME.tile_size + 32;
      d->texture_index = id;
      d->collected = 0;
    }
  }
}

void add_tile (int x, int y)
{
  int i, idx, flags = 0xf;

  if (GAME.tile_count >= MAX_TILES) {
    return;
  }

  for (i = 0; i < GAME.tile_count; i++) {
    if (GAME.tiles[i].x == x && GAME.tiles[i].y == y) {
      return;
    }
    if (GAME.tiles[i].y == y && GAME.tiles[i].x == (x - 1)) {
      flags &= ~1;
      GAME.tiles[i].flags &= ~(1 << 2);
    } else if (GAME.tiles[i].x == x && GAME.tiles[i].y == (y + 1)) {
      flags &= ~(1 << 1);
      GAME.tiles[i].flags &= ~(1 << 3);
    } else if (GAME.tiles[i].y == y && GAME.tiles[i].x == (x + 1)) {
      flags &= ~(1 << 2);
      GAME.tiles[i].flags &= ~1;
    } else if (GAME.tiles[i].x == x && GAME.tiles[i].y == (y - 1)) {
      flags &= ~(1 << 3);
      GAME.tiles[i].flags &= ~(1 << 1);
    }
  }

  idx = GAME.tile_count++;

  GAME.tiles[idx].x = x;
  GAME.tiles[idx].y = y;
  GAME.tiles[idx].flags = flags;

  add_dirt_pile(x, y, GAME.dust_texture_index);
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

void draw_hud (Player *p)
{
  char score_string[20];

  m8_fill_color(105, 106, 106);
  m8_fill_rect(20, 23, 5, 9);
  m8_fill_rect(25, 20, 30, 15);
  m8_fill_color(106, 190, 48);

  if (p->charge > 1.0) m8_fill_rect(27, 22, 5, 11);
  if (p->charge > 2.0) m8_fill_rect(34, 22, 5, 11);
  if (p->charge > 3.0) m8_fill_rect(41, 22, 5, 11);
  if (p->charge > 4.0) m8_fill_rect(48, 22, 5, 11);

  sprintf(score_string, "SCORE: %d", p->score);
  m8_draw_text_8x8(ascii, score_string, (int)strlen(score_string), 20, 315);
}

void init_game (int width, int height)
{
  GAME.room_count = 0;
  GAME.tile_count = 0;
  GAME.screen_width = width;
  GAME.screen_height = height;
  GAME.tile_size = 64;
  GAME.dirt_count = 0;

  generate_world();
}

void load_texture (const char *file, int *start, int *end)
{
  int tw, th, channels;
  unsigned char *bmp = stbi_load(file, &tw, &th, &channels, 0);
  m8_import_tiles_from_image(bmp, tw, th, channels, 32, start, end);
  free(bmp);
}

void run (window_t *window, m8u32 *framebuffer, int width, int height)
{
  Mode8 *ctx;
  int start_index, end_index;
  Player robo;
  int has_begun = 0;

  ctx = malloc(sizeof(Mode8));

  m8_use_context(ctx);
  m8_init(framebuffer, width, height);

  init_player(&robo);

  load_texture("images/robovac-sheet.png", &start_index, &end_index);
  robo.texture_start_index = start_index;
  robo.texture_end_index = end_index;

  load_texture("images/floortile.png", &start_index, &end_index);
  GAME.floor_texture_index = start_index;

  load_texture("images/walltile.png", &start_index, &end_index);
  GAME.wall_texture_index = start_index;

  load_texture("images/dust.png", &start_index, &end_index);
  GAME.dust_texture_index = start_index;

  init_game(width, height);

  while (!window->quit) {
    if (has_begun) {
      if (window->keys.w) {
        robo.is_accelerating = 1;
      } else {
        robo.is_accelerating = 0;
      }

      if (robo.speed <= 0) {
        if (window->keys.a) {
          rotate_player(&robo, 0.25);
        } else if (window->keys.d) {
          rotate_player(&robo, -0.25);
        }
      }
    } else {
      if (window->keys.enter) {
        has_begun = 1;
      }
    }

    if (has_begun) {
      if (robo.charge <= 0) {
        const char* message = "Your battery has died";
        const char* begin = "press ENTER to continue";
        char score[20];
        sprintf(score, "SCORE: %d", robo.score);
        m8_draw_text_8x8(ascii, message, strlen(message), 235, 150);
        m8_draw_text_8x8(ascii, score, strlen(score), 280, 167);
        m8_draw_text_8x8(ascii, begin, strlen(begin), 230, 190);

        if (window->keys.enter) {
          init_player(&robo);
          init_game(width, height);
        }
      } else {
        update_player(&robo);

        GAME.camera.x = robo.x + (robo.tile_size / 2);
        GAME.camera.y = robo.y + (robo.tile_size / 2);

        clean_dirt(&robo);

        draw_world();
        draw_dirt();
        draw_player(&robo);
        draw_hud(&robo);
      }
    } else {
      const char* title = "ROBO-VAC 9000 TESTING FACILITY";
      const char* desc1 = "Before being adopted by a human";
      const char* desc2 = "you will learn how to clean a human home";
      const char* begin = "press ENTER to continue";
      m8_draw_text_8x8(ascii, title, strlen(title), 200, 150);
      m8_draw_text_8x8(ascii, desc1, strlen(desc1), 200, 175);
      m8_draw_text_8x8(ascii, desc2, strlen(desc2), 165, 185);
      m8_draw_text_8x8(ascii, begin, strlen(begin), 230, 215);
    }

    window_update(window, framebuffer);
    m8_clear();
  }

  window_close(window);
  free(framebuffer);
}

int main (void) {
  int width = 640, height = 360;
  m8u32* framebuffer;
  window_t window;

  srand(time(NULL));

  if (!(framebuffer = (m8u32 *)malloc((width * height) * sizeof(m8u32)))) {
    return 1;
  }

  window_open(&window, "game", width, height, 1280, 720);
  run(&window, framebuffer, width, height);
}
