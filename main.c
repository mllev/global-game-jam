#include <stdio.h>
#include <time.h>
#include <string.h>

#include "font.h"

#define WINDOW_IMPLEMENTATION
#include "window.h"

#define GFX_IMPLEMENTATION
#include "gfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct vec2 Vec2;
typedef struct player Player;

struct vec2 { float x, y; };

struct player {
  float x, y;
  Vec2 forward;
  float speed;
  int texture_id;
  float angle;
};

void init_player (Player *p)
{
  p->x = 0;
  p->y = 0;
  p->forward.x = 0;
  p->forward.y = -1;
  p->speed = 0.15;
  p->angle = 0.0;
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

  if (p->angle > (GFX_PI * 2)) {
    p->angle -= (GFX_PI * 2);
  } else if (p->angle < -(GFX_PI * 2)) {
    p->angle += (GFX_PI * 2);
  }
}

void draw_player (Player* p)
{
  float max = GFX_PI * 2.0;
  float tex_frac = 1.0 / 22.0;
  float frac = floor((max - p->angle) / max * 22.0) * tex_frac;

  float vertices[] = {
    0.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 0.0
  };

  int indices[] = {
    0, 1, 2,
    0, 2, 3
  };

  float uvs[] = {
    1.0, 1.0,
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
  };

  uvs[0] = frac + tex_frac;
  uvs[2] = frac + tex_frac;
  uvs[4] = frac;
  uvs[6] = frac;

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(p->x, p->y, 10);
  gfx_scale(3, 3, 0);
  gfx_translate(-0.5, -0.5, 0);
  gfx_bind_arrays(vertices, 4, indices, 2);
  gfx_bind_attr(GFX_ATTR_UVS, uvs);
  gfx_bind_texture(p->texture_id);
  gfx_draw_arrays(0, -1);
}

void run (window_t *window, gfxuint *framebuffer, int width, int height)
{
  float *depthbuffer = malloc(width * height * sizeof(float));
  gfx_context *ctx = malloc(sizeof(gfx_context));
  int tw, th, channels;
  unsigned char *bmp;
  int texture_id;
  Player robo;

  bmp = stbi_load("robovac-sheet.png", &tw, &th, &channels, 0);

  if (!depthbuffer) {
    free(framebuffer);
    return;
  }

  init_player(&robo);

  gfx_use_context(ctx);
  gfx_init();
  gfx_bind_render_target(framebuffer, width, height);
  gfx_bind_depth_buffer(depthbuffer);
  gfx_matrix_mode(GFX_PROJECTION_MATRIX);
  gfx_identity();
  gfx_perspective(70, (float)width / (float)height, 0.1, 100);
  gfx_upload_texture(bmp, tw, th, channels, &texture_id);

  robo.texture_id = texture_id;

  while (!window->quit) {
    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();

    if (window->keys.w) {
      robo.x += (robo.forward.x * robo.speed);
      robo.y += (robo.forward.y * robo.speed);
    } else if (window->keys.a) {
      rotate_player(&robo, 0.1);
    } else if (window->keys.d) {
      rotate_player(&robo, -0.1);
    }

    draw_player(&robo);

    window_update(window, framebuffer);
    gfx_clear();
  }

  window_close(window);
  free(framebuffer);
}

int main (void) {
  int width = 1280, height = 720;
  gfxuint* framebuffer;
  window_t window;

  if (!(framebuffer = (gfxuint *)malloc((width * height) * sizeof(gfxuint)))) {
    return 1;
  }

  window_open(&window, "game", width, height, 1280, 720);
  run(&window, framebuffer, width, height);
}
