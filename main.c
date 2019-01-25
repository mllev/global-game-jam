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

void run (window_t *window, gfxuint *framebuffer, int width, int height)
{
  float *depthbuffer = malloc(width * height * sizeof(float));
  float white[] = { 1.0, 1.0, 1.0 };
  float rotate = 0.0;
  gfx_context *ctx = malloc(sizeof(gfx_context));
  int tw, th, channels;
  unsigned char *bmp;
  int texture_id;

  float cube_vertices[] = {
    0.0, 0.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 1.0,
    1.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 1.0
  };

  int cube_indices[] = {
    0, 6, 4,
    0, 2, 6,
    0, 3, 2,
    0, 1, 3,
    2, 7, 6,
    2, 3, 7,
    4, 7, 5,
    4, 6, 7,
    0, 5, 1,
    0, 4, 5,
    1, 7, 3,
    1, 5, 7
  };

  bmp = stbi_load("images.png", &tw, &th, &channels, 0);

  if (!depthbuffer) {
    free(framebuffer);
    return;
  }

  gfx_use_context(ctx);
  gfx_init();
  gfx_bind_render_target(framebuffer, width, height);
  gfx_bind_depth_buffer(depthbuffer);
  gfx_matrix_mode(GFX_PROJECTION_MATRIX);
  gfx_identity();
  gfx_perspective(70, (float)width / (float)height, 0.1, 100);
  gfx_upload_texture(bmp, tw, th, channels, &texture_id);

  while (!window->quit) {
    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();

    gfx_point_light(-3, 2, 0, 1, 0, 0, 1);
    gfx_point_light(0, 0, 0, 0, 0, 1, 1);

    gfx_matrix_mode(GFX_MODEL_MATRIX);

    gfx_identity();
    gfx_translate(0, 0, 3);
    gfx_rotate(0, 1, 0, rotate);
    gfx_rotate(1, 0, 0, rotate);
    gfx_translate(-0.5, -0.5, -0.5);
    gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
    gfx_bind_attr(GFX_ATTR_RGB, white);
    gfx_draw_arrays(0, -1);

    gfx_identity();
    gfx_translate(-3, 0, 3);
    gfx_rotate(0, 0, 1, rotate);
    gfx_draw_textured_quad(texture_id);

    window_update(window, framebuffer);
    gfx_clear();
    gfx_reset_lights();
    rotate += 0.01;
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
