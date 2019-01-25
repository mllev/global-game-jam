#ifndef _WINDOW_H
#define _WINDOW_H

#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct _window window_t;

void window_open(window_t*, const char*, int, int, int, int);
void window_update(window_t*, unsigned*);
void window_close(window_t*);

struct _window {
  struct {
    int a, w, s, d, p, m;
    int up, down, left, right;
    int enter;
    int _1, _2, _3, _4;
  } keys;

  struct {
    int x, y;
  } mouse;

  int quit;

  SDL_Event event;
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;

  int target_width;
  int target_height;
};

#ifdef WINDOW_IMPLEMENTATION

void window_open (window_t *w, const char *name, int width, int height, int real_width, int real_height)
{
  int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  int pflags = SDL_PIXELFORMAT_ARGB8888;
  int tflags = SDL_TEXTUREACCESS_STREAMING;

  SDL_Init(SDL_INIT_VIDEO);
  
  memset(w, 0, sizeof(window_t));
  w->window = SDL_CreateWindow(name, 10, 10, width, height, 0);

  SDL_SetWindowSize(w->window, real_width, real_height);

  w->renderer = SDL_CreateRenderer(w->window, -1, flags);
  w->texture = SDL_CreateTexture(w->renderer, pflags, tflags, width, height);

  w->target_width = width;
  w->target_height = height;
}

void window_update (window_t *w, unsigned *buf)
{
  SDL_Delay(10);
  SDL_UpdateTexture(w->texture, NULL, buf, w->target_width * sizeof(unsigned));
  SDL_RenderClear(w->renderer);
  SDL_RenderCopy(w->renderer, w->texture, NULL, NULL);
  SDL_RenderPresent(w->renderer);

  while (SDL_PollEvent(&(w->event))) {
    int down = 1;

    switch (w->event.type) {
      case SDL_QUIT:
        w->quit = 1;
        break;
      case SDL_KEYUP:
        down = 0;
      case SDL_KEYDOWN:
        switch (w->event.key.keysym.sym) {
          case SDLK_a:     w->keys.a     = down; break;
          case SDLK_d:     w->keys.d     = down; break;
          case SDLK_w:     w->keys.w     = down; break;
          case SDLK_s:     w->keys.s     = down; break;
          case SDLK_p:     w->keys.p     = down; break;
          case SDLK_m:     w->keys.m     = down; break;
          case SDLK_1:     w->keys._1    = down; break;
          case SDLK_2:     w->keys._2    = down; break;
          case SDLK_3:     w->keys._3    = down; break;
          case SDLK_4:     w->keys._4    = down; break;
          case SDLK_UP:    w->keys.up    = down; break;
          case SDLK_DOWN:  w->keys.down  = down; break;
          case SDLK_LEFT:  w->keys.left  = down; break;
          case SDLK_RIGHT: w->keys.right = down; break;
          case SDLK_RETURN: w->keys.enter = down; break;
          default: break;
        }
        break;
      default: break;
    }
  }
}

void window_close (window_t *w)
{
  SDL_DestroyTexture(w->texture);
  SDL_DestroyRenderer(w->renderer);
  SDL_DestroyWindow(w->window);
  SDL_Quit();
}

#endif
#endif
