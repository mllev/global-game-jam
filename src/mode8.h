#ifndef _M8_H
#define _M8_H

#define M8_PI 3.141592653589793
#define M8_MAX_SPRITES 1024

typedef unsigned int m8u32;
typedef m8u32 m8argb32;

typedef struct mode8 Mode8;
typedef struct m8sprite8x8 m8Sprite8x8;
typedef struct m8sprite16x16 m8Sprite16x16;
typedef struct m8sprite32x32 m8Sprite32x32;

struct m8sprite8x8 {
  char data[64];
};

struct m8sprite16x16 {
  char data[256];
};

struct m8sprite32x32 {
  char data[1024];
};

struct mode8 {
  m8argb32 *framebuffer;
  m8argb32 palette[256];
  m8argb32 fill_color;
  m8argb32 clear_color;

  m8Sprite8x8   sprites_8x8[M8_MAX_SPRITES];
  m8Sprite16x16 sprites_16x16[M8_MAX_SPRITES];
  m8Sprite32x32 sprites_32x32[M8_MAX_SPRITES];

  int num_8x8_sprites;
  int num_16x16_sprites;
  int num_32x32_sprites;

  int width;
  int height;

  int num_palette_colors;
};

Mode8* _MODE8;

#define MODE8 (*_MODE8)

void m8_init(m8u32*, int, int);
void m8_draw_text_8x8(char[][8], const char *, int, int, int);
unsigned char m8_add_to_palette(int, int, int, int);
void m8_fill_rect(float, float, float, float);
void m8_draw_sprite (float, float, float, float, unsigned char*, int, int);
void m8_draw_tile (float, float, int, int);
void m8_draw_tile_scaled (float, float, float, float, int, int);
void m8_clear(void);
void m8_use_context(Mode8*);
void m8_fill_color(int, int, int);
void m8_clear_color(int, int, int);
void m8_import_tiles_from_image(unsigned char*, int, int, int, int, int*, int*);

#ifdef M8_IMPLEMENTATION

void m8_init (m8u32 *framebuffer, int width, int height)
{
  MODE8.framebuffer = framebuffer;
  MODE8.width = width;
  MODE8.height = height;
  MODE8.num_palette_colors = 1;
  MODE8.fill_color = 0xffffffff;
  MODE8.clear_color = 0;
  MODE8.num_8x8_sprites = 0;
  MODE8.num_16x16_sprites = 0;
  MODE8.num_32x32_sprites = 0;
}

void m8_use_context (Mode8 *ctx)
{
  _MODE8 = ctx;
}

void m8_clear ()
{
  int i, l = MODE8.width * MODE8.height;
  for (i = 0; i < l; i++) MODE8.framebuffer[i] = MODE8.clear_color;
}

unsigned char m8_add_to_palette (int r, int g, int b, int a)
{
  unsigned char i;
  unsigned int color = (255 << 24) | (r << 16) | (g << 8) | b;

  if (a < 255) {
    return 0;
  }

  for (i = 1; i < MODE8.num_palette_colors; i++) {
    if (MODE8.palette[i] == color) {
      return i;
    }
  }

  MODE8.palette[i] = color;
  return MODE8.num_palette_colors < 255 ? MODE8.num_palette_colors++ : 255;
}

void m8_import_tiles_from_image (unsigned char *image, int width, int height, int channels, int tilesize, int *start, int *end)
{
  unsigned char *ptr, *tile;
  int stride = (width - tilesize) * channels;
  int max = tilesize * tilesize;
  int tiles_x = width / tilesize;
  int tiles_y = height / tilesize;
  int i, x, y;

  *start = MODE8.num_32x32_sprites;

  for (y = 0; y < tiles_y; y++) {
    for (x = 0; x < tiles_x; x++) {
      int real_x = x * tilesize;
      int real_y = y * tilesize;

      switch (tilesize) {
        case 32:
          tile = (unsigned char *)MODE8.sprites_32x32[MODE8.num_32x32_sprites++].data;
          break;
        case 16:
          tile = (unsigned char *)MODE8.sprites_16x16[MODE8.num_16x16_sprites++].data;
          break;
        default:
          break;
      }

      ptr = image + (real_y * (width * channels) + (real_x * channels));

      for (i = 0; i < max; i++) {
        int r = *(ptr++);
        int g = *(ptr++);
        int b = *(ptr++);
        int a = channels == 4 ? *(ptr++) : 255;

        tile[i] = m8_add_to_palette(r, g, b, a);

        if (((i+1)%tilesize) == 0) {
          ptr += stride;
        }
      }
    }
  }

  *end = MODE8.num_32x32_sprites - 1;
}

void m8_invert_y (int *y1, int *y2)
{
  int tmp;

  *y1 = MODE8.height - 1 - *y1;
  *y2 = MODE8.height - 1 - *y2;
  tmp = *y1;
  *y1 = *y2;
  *y2 = tmp;
}

void m8_fill_rect (float x, float y, float w, float h)
{
  int start_x = (int)round(x);
  int start_y = (int)round(y);
  int width = (int)round(w);
  int height = (int)round(h);
  int end_x = start_x + width - 1;
  int end_y = start_y + height - 1;

  int count;

  unsigned int *dst, *buf;

  if (start_x >= MODE8.width || start_y >= MODE8.height || end_x < 0 || end_y < 0) {
    return;
  }

  m8_invert_y(&start_y, &end_y);

  if (start_x < 0) start_x = 0;
  if (start_y < 0) start_y = 0;

  if (end_x >= MODE8.width)  end_x = MODE8.width - 1;
  if (end_y >= MODE8.height) end_y = MODE8.height - 1;

  buf = MODE8.framebuffer + (MODE8.width * start_y + start_x);

  while (start_y <= end_y) {
    dst = buf;
    count = end_x + 1 - start_x;

    while (count--) {
      *(dst++) = MODE8.fill_color;
    }

    buf += MODE8.width;
    start_y++;
  }
}

void m8_draw_tile (float x, float y, int type, int index)
{
  switch (type) {
    case 32:
      m8_draw_sprite(x, y, 32, 32, (unsigned char *)MODE8.sprites_32x32[index].data, 32, 32);
      break;
    case 16:
      m8_draw_sprite(x, y, 16, 16, (unsigned char *)MODE8.sprites_16x16[index].data, 16, 16);
      break;
    default:
      break;
  }
}

void m8_draw_tile_scaled (float x, float y, float width, float height, int type, int index)
{
  switch (type) {
    case 32:
      m8_draw_sprite(x, y, width, height, (unsigned char *)MODE8.sprites_32x32[index].data, 32, 32);
      break;
    case 16:
      m8_draw_sprite(x, y, width, height, (unsigned char *)MODE8.sprites_16x16[index].data, 16, 16);
      break;
    default:
      break;
  }
}

void m8_draw_tile_range (int index, int width, int height, float x, float y, int size)
{
  int i, j;
  for (j = 0; j < width; j++) {
    for (i = 0; i < height; i++) {
      m8_draw_tile(x + (float)(size * i), y + (float)(size * j), size, index++);
    }
  }
}

void m8_draw_sprite (float x, float y, float w, float h, unsigned char *img, int tw, int th)
{
  int start_x = (int)floor(x);
  int start_y = (int)floor(y);
  int width = (int)floor(w);
  int height = (int)floor(h);
  int tmp;

  int end_x = start_x + width - 1;
  int end_y = start_y + height - 1;

  int count;
  int stride = MODE8.width;

  unsigned int *dst, *buf;

  m8u32 u_fixed, u_start_fixed, u_step_fixed;
  m8u32 v, v_start_fixed, v_step_fixed;

  m8_invert_y(&start_y, &end_y);

  u_start_fixed = 0;
  v_start_fixed = 0;

  u_step_fixed = (tw << 16) / width;
  v_step_fixed = (th << 16) / height;

  if (start_x >= MODE8.width || start_y >= MODE8.height || end_x < 0 || end_y < 0) {
    return;
  }

  if (start_x < 0) {
    u_start_fixed += (u_step_fixed * -start_x);
    start_x = 0;
  }

  if (start_y < 0) {
    v_start_fixed += (v_step_fixed * -start_y); 
    start_y = 0;
  }

  if (end_x >= MODE8.width)  end_x = MODE8.width - 1;
  if (end_y >= MODE8.height) end_y = MODE8.height - 1;

  buf = MODE8.framebuffer + (MODE8.width * start_y + start_x);
  width = (end_x - start_x) + 1;

  #define PIXEL() { \
    int idx = img[v + (u_fixed >> 16)]; \
    if (idx) *dst = MODE8.palette[idx]; \
    u_fixed += u_step_fixed; \
    dst++; \
  }

  while (start_y <= end_y) {
    dst = buf;
    u_fixed = u_start_fixed;
    count = width;
    v = (v_start_fixed >> 16) * tw;

    while (count > 8) {
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      count -= 8;
    }

    while (count > 4) {
      PIXEL();
      PIXEL();
      PIXEL();
      PIXEL();
      count -= 4;
    }

    while (count--) {
      PIXEL();
    }

    buf += stride;
    start_y++;
    v_start_fixed += v_step_fixed;
  }

  #undef PIXEL
}

void m8_fill_color (int r, int g, int b)
{
  MODE8.fill_color = (255 << 24) | (r << 16) | (g << 8) | b;
}

void m8_clear_color (int r, int g, int b)
{
  MODE8.clear_color = (255 << 24) | (r << 16) | (g << 8) | b;
}

void m8_draw_text_8x8 (char font[][8], const char *str, int length, int offx, int offy)
{
  int i, y, x, l, min = 0, max = length;
  m8u32 c;
  m8u32 *buf = MODE8.framebuffer;
  int w = MODE8.width;
  int h = MODE8.height;

  for (y = 0; y < 8; y++) {
    int idx = (offy + y) * w;
    x = offx;
    if (offy + y < 0 || offy + y >= h) continue;
    for (l = min; l < max; l++) {
      char* letter = (char *)font[(int)str[l]];
      char bits = letter[y];
      for (i = 0; i < 8; i++) {
        if (bits & (1 << i)) c = 0xffffffff; else c = 0;
        if (x >= 0 && x < w) buf[idx + (x++)] = c; else x++;
      }
    }
  }
}

#endif
#endif
