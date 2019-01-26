#ifndef _GFX_H
#define _GFX_H

#include <math.h> /* sinf cosf fabs */

#define GFX_PI 3.141592653589793

#ifndef GFX_MAX_VERTICES
#define GFX_MAX_VERTICES 10000
#endif

#ifndef GFX_MAX_FACES
#define GFX_MAX_FACES 10000
#endif

#ifndef GFX_MAX_TEXTURES
#define GFX_MAX_TEXTURES 100
#endif

#ifndef GFX_TEXTURE_CACHE_SIZE
#define GFX_TEXTURE_CACHE_SIZE 4000000
#endif

#define gfx_max2(x, y) ((x) > (y) ? (x) : (y))
#define gfx_min2(x, y) ((x) < (y) ? (x) : (y))
#define gfx_min3(x, y, z) ((x) < (y) ? gfx_min2(x, z) : gfx_min2(y, z))
#define gfx_max3(x, y, z) ((x) > (y) ? gfx_max2(x, z) : gfx_max2(y, z))
#define gfx_clamp(x, y, z) (gfx_max2(gfx_min2(x, z), y))

typedef unsigned int gfxuint;
typedef unsigned char gfxbyte;

typedef struct _gfx gfx_context;

typedef struct _gfxuv gfxuv;
typedef struct _gfxv2 gfxv2;
typedef struct _gfxv4 gfxv4;
typedef struct _gfxm4 gfxm4;

typedef struct _gfxrgb gfxrgb;

typedef struct _gfxvert gfxvert;
typedef struct _gfxpoly gfxpoly;
typedef struct _gfxedge gfxedge;

typedef gfxv4 gfxnormal;

struct _gfxuv { float u, v; };
struct _gfxv2 { float x, y; };
struct _gfxv4 { float x, y, z, w; };

struct _gfxm4 {
  float _00, _01, _02, _03;
  float _10, _11, _12, _13;
  float _20, _21, _22, _23;
  float _30, _31, _32, _33;
};

struct _gfxrgb {
  float r, g, b;
};

struct _gfxvert {
  gfxv4 transformed;
  gfxnormal normal;
  gfxv4 screen_space;
  gfxuv uv;
  gfxbyte clip_flags;
};

struct _gfxpoly {
  int verts[3];
  gfxv2 projected[3];
  gfxrgb color;
  gfxnormal normal;
  gfxv4 center;
};

struct _gfxedge {
  float x, x_step;
  float z, z_step;
  float u, u_step;
  float v, v_step;
  int y_start;
  int y_end;
};

struct _gfx {
  gfxuint* target;
  float *depth_buffer;

  int target_width;
  int target_height;

  gfxbyte *texture;
  gfxrgb palette[256];

  gfxuint palette_argb32[256];
  int num_palette_colors;
  int texture_width;
  int texture_height;

  float *vertices;
  int *indices;
  float *colors;
  float *uvs;

  int attr_type;

  int enabled;

  gfxrgb solid_color;

  struct {
    int type;
    gfxv4 position;
    gfxrgb color;
    float intensity;
  } lights[8];

  int num_lights;

  gfxuint clear_color;

  int vertex_count;
  int index_count;

  int draw_mode;

  gfxm4 model;
  gfxm4 view;
  gfxm4 projection;
  gfxm4 mvp;

  gfxm4* active;

  gfxvert vertex_pipe[GFX_MAX_VERTICES];
  gfxpoly visible[GFX_MAX_FACES];

  gfxbyte texture_cache[GFX_TEXTURE_CACHE_SIZE];

  struct {
    int index;
    int width, height;
  } textures[GFX_MAX_TEXTURES];

  int texture_count;
  int current_texture_index;
};

enum {
  GFX_ATTR_RGB,
  GFX_ATTR_COLORS,
  GFX_ATTR_UVS
};

enum {
  GFX_MODEL_MATRIX,
  GFX_VIEW_MATRIX,
  GFX_PROJECTION_MATRIX
};

enum {
  GFX_FLAT_FILL_MODE,
  GFX_WIREFRAME_MODE
};

enum {
  GFX_DIRECTIONAL_LIGHT,
  GFX_POINT_LIGHT
};

enum {
  GFX_LIGHTS = 1
};

int gfx_init(void);
unsigned int gfx_memory_requirements(void);

void gfx_clear(void);
void gfx_clear_color(float, float, float);
void gfx_point_light(float, float, float, float, float, float, float);
void gfx_directional_light(float, float, float, float, float, float, float);
void gfx_reset_lights(void);
void gfx_bind_render_target(gfxuint*, int, int);
void gfx_bind_depth_buffer(float*);
void gfx_bind_arrays(float*, int, int*, int);
void gfx_bind_attr(int, float*);
void gfx_bind_texture(int);
void gfx_draw_arrays(int, int);
void gfx_draw_text_8x8(char[][8], const char *, int, int, int);
void gfx_draw_line(float, float, float, float, gfxuint);
void gfx_enable(int);
void gfx_disable(int);
void gfx_draw_mode(int);
void gfx_matrix_mode(int);
void gfx_rotate(float, float, float, float);
void gfx_translate(float, float, float);
void gfx_scale(float, float, float);
void gfx_identity(void);
void gfx_perspective(float, float, float, float);

#ifdef GFX_IMPLEMENTATION

static struct _gfx *__GFX;

#define GFX (*__GFX)

#ifdef GFX_DEBUG

void gfx_m4_print (gfxm4 *m)
{
  printf("%f %f %f %f\n", m->_00, m->_01, m->_02, m->_03);
  printf("%f %f %f %f\n", m->_10, m->_11, m->_12, m->_13);
  printf("%f %f %f %f\n", m->_20, m->_21, m->_22, m->_23);
  printf("%f %f %f %f\n", m->_30, m->_31, m->_32, m->_33);
  printf("\n");
}

void gfx_v4_print (gfxv4 *v)
{
  printf("%f %f %f %f\n\n", v->x, v->y, v->z, v->w);
}

#endif

void gfx_m4_mult (gfxm4 *r, gfxm4 *m1, gfxm4 *m2)
{
  r->_00 = m1->_00 * m2->_00 + m1->_01 * m2->_10 + m1->_02 * m2->_20 + m1->_03 * m2->_30;
  r->_01 = m1->_00 * m2->_01 + m1->_01 * m2->_11 + m1->_02 * m2->_21 + m1->_03 * m2->_31;
  r->_02 = m1->_00 * m2->_02 + m1->_01 * m2->_12 + m1->_02 * m2->_22 + m1->_03 * m2->_32;
  r->_03 = m1->_00 * m2->_03 + m1->_01 * m2->_13 + m1->_02 * m2->_23 + m1->_03 * m2->_33;

  r->_10 = m1->_10 * m2->_00 + m1->_11 * m2->_10 + m1->_12 * m2->_20 + m1->_13 * m2->_30;
  r->_11 = m1->_10 * m2->_01 + m1->_11 * m2->_11 + m1->_12 * m2->_21 + m1->_13 * m2->_31;
  r->_12 = m1->_10 * m2->_02 + m1->_11 * m2->_12 + m1->_12 * m2->_22 + m1->_13 * m2->_32;
  r->_13 = m1->_10 * m2->_03 + m1->_11 * m2->_13 + m1->_12 * m2->_23 + m1->_13 * m2->_33;

  r->_20 = m1->_20 * m2->_00 + m1->_21 * m2->_10 + m1->_22 * m2->_20 + m1->_23 * m2->_30;
  r->_21 = m1->_20 * m2->_01 + m1->_21 * m2->_11 + m1->_22 * m2->_21 + m1->_23 * m2->_31;
  r->_22 = m1->_20 * m2->_02 + m1->_21 * m2->_12 + m1->_22 * m2->_22 + m1->_23 * m2->_32;
  r->_23 = m1->_20 * m2->_03 + m1->_21 * m2->_13 + m1->_22 * m2->_23 + m1->_23 * m2->_33;

  r->_30 = m1->_30 * m2->_00 + m1->_31 * m2->_10 + m1->_32 * m2->_20 + m1->_33 * m2->_30;
  r->_31 = m1->_30 * m2->_01 + m1->_31 * m2->_11 + m1->_32 * m2->_21 + m1->_33 * m2->_31;
  r->_32 = m1->_30 * m2->_02 + m1->_31 * m2->_12 + m1->_32 * m2->_22 + m1->_33 * m2->_32;
  r->_33 = m1->_30 * m2->_03 + m1->_31 * m2->_13 + m1->_32 * m2->_23 + m1->_33 * m2->_33;
}

void gfx_v4_mult (gfxv4 *r, gfxv4 *v, gfxm4 *m)
{
  r->x = m->_00 * v->x + m->_01 * v->y + m->_02 * v->z + m->_03 * v->w;
  r->y = m->_10 * v->x + m->_11 * v->y + m->_12 * v->z + m->_13 * v->w;
  r->z = m->_20 * v->x + m->_21 * v->y + m->_22 * v->z + m->_23 * v->w;
  r->w = m->_30 * v->x + m->_31 * v->y + m->_32 * v->z + m->_33 * v->w;
}

void gfx_v4_init (gfxv4 *v, float x, float y, float z, float w)
{
  v->x = x;
  v->y = y;
  v->z = z;
  v->w = w;
}

void gfx_m4_ident (gfxm4 *m)
{
  m->_00 = 1; m->_01 = 0; m->_02 = 0; m->_03 = 0;
  m->_10 = 0; m->_11 = 1; m->_12 = 0; m->_13 = 0;
  m->_20 = 0; m->_21 = 0; m->_22 = 1; m->_23 = 0;
  m->_30 = 0; m->_31 = 0; m->_32 = 0; m->_33 = 1;
}

void gfx_m4_rotation (gfxm4 *m, float x, float y, float z, float a)
{
  float s = sinf(a);
  float c = cosf(a);

  float c1 = 1 - c;

  float xy = x * y, xx = x * x, xz = x * z;
  float yy = y * y, yz = y * z, zz = z * z;
  float xs = x * s, ys = y * s, zs = z * s;

  m->_00 = xx * c1 + c;  m->_01 = xy * c1 - zs; m->_02 = xz * c1 + ys; m->_03 = 0;
  m->_10 = xy * c1 + zs; m->_11 = yy * c1 + c;  m->_12 = yz * c1 - xs; m->_13 = 0;
  m->_20 = xz * c1 - ys; m->_21 = yz * c1 + xs; m->_22 = zz * c1 + c;  m->_23 = 0;
  m->_30 = 0;            m->_31 = 0;            m->_32 = 0;            m->_33 = 1;
}

void gfx_m4_scale (gfxm4 *m, float x, float y, float z)
{
  m->_00 = x;  m->_01 = 0;  m->_02 = 0;  m->_03 = 0;
  m->_10 = 0;  m->_11 = y;  m->_12 = 0;  m->_13 = 0;
  m->_20 = 0;  m->_21 = 0;  m->_22 = z;  m->_23 = 0;
  m->_30 = 0;  m->_31 = 0;  m->_32 = 0;  m->_33 = 1;
}

void gfx_m4_translation (gfxm4 *m, float x, float y, float z)
{
  m->_00 = 1;  m->_01 = 0;  m->_02 = 0;  m->_03 = x;
  m->_10 = 0;  m->_11 = 1;  m->_12 = 0;  m->_13 = y;
  m->_20 = 0;  m->_21 = 0;  m->_22 = 1;  m->_23 = z;
  m->_30 = 0;  m->_31 = 0;  m->_32 = 0;  m->_33 = 1;
}

void gfx_m4_perspective (gfxm4 *m, float fov, float ar, float nearz, float farz)
{
  float f = 1.0 / tanf(fov / 2);
  float range = nearz - farz;

  float a = f / ar;
  float b = -(nearz + farz) / range;
  float c = (2 * farz * nearz) / range;

  m->_00 = a;  m->_01 = 0;  m->_02 = 0;  m->_03 = 0;
  m->_10 = 0;  m->_11 = f;  m->_12 = 0;  m->_13 = 0;
  m->_20 = 0;  m->_21 = 0;  m->_22 = b;  m->_23 = c;
  m->_30 = 0;  m->_31 = 0;  m->_32 = 1;  m->_33 = 0;
}

float gfx_v4_length (gfxv4 *v)
{
  return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

void gfx_v4_normalize (gfxv4 *out, gfxv4 *in)
{
  float m = gfx_v4_length(in);
  float d = 1 / m;

  out->x = in->x * d;
  out->y = in->y * d;
  out->z = in->z * d;
}

void gfx_v4_crossp (gfxv4 *out, gfxv4 *a, gfxv4 *b)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}

float gfx_v2_area (gfxv2* v1, gfxv2* v2, gfxv2* v3)
{
  return ((v2->x - v1->x) * (v3->y - v1->y) - (v3->x - v1->x) * (v2->y - v1->y));
}

float gfx_v4_dotp (gfxv4 *v1, gfxv4 *v2)
{
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

void gfx_v4_sub (gfxv4 *out, gfxv4 *v1, gfxv4 *v2)
{
  out->x = v1->x - v2->x;
  out->y = v1->y - v2->y;
  out->z = v1->z - v2->z;
  out->w = v1->w - v2->w;
}

void gfx_v4_add (gfxv4 *out, gfxv4 *v1, gfxv4 *v2)
{
  out->x = v1->x + v2->x;
  out->y = v1->y + v2->y;
  out->z = v1->z + v2->z;
  out->w = v1->w + v2->w;
}

void gfx_m4_copy (gfxm4 *a, gfxm4 *b)
{
  a->_00 = b->_00; a->_01 = b->_01; a->_02 = b->_02; a->_03 = b->_03;
  a->_10 = b->_10; a->_11 = b->_11; a->_12 = b->_12; a->_13 = b->_13;
  a->_20 = b->_20; a->_21 = b->_21; a->_22 = b->_22; a->_23 = b->_23;
  a->_30 = b->_30; a->_31 = b->_31; a->_32 = b->_32; a->_33 = b->_33;
}

gfxbyte gfx_add_to_palette (float r, float g, float b, float a)
{
  int i;

  if (a < 1.0) {
    return 0;
  }

  for (i = 1; i < GFX.num_palette_colors; i++) {
    if (GFX.palette[i].r == r && GFX.palette[i].g == g && GFX.palette[i].b == b) {
      return i;
    }
  }

  if (GFX.num_palette_colors < 256) {
    int idx = GFX.num_palette_colors++;
    gfxrgb c;

    c.r = r;
    c.g = g;
    c.b = b;

    GFX.palette[idx] = c;

    return idx;
  } else {
    return 255;
  }
}

void gfx_enable (int feature)
{
  GFX.enabled |= feature;
}

void gfx_disable (int feature)
{
  GFX.enabled &= ~feature;
}

void gfx_upload_texture (unsigned char *image, int width, int height, int channels, int *id)
{
  gfxbyte *ptr = image;
  int i, max = width * height;
  int tidx = GFX.texture_count++;

  GFX.textures[tidx].width = width;
  GFX.textures[tidx].height = height;
  GFX.textures[tidx].index = GFX.current_texture_index;

  for (i = 0; i < max; i++) {
    float r = (float)(*(ptr++)) / 255.0;
    float g = (float)(*(ptr++)) / 255.0;
    float b = (float)(*(ptr++)) / 255.0;
    float a = channels == 4 ? (*(ptr++) / 255.0) : 1.0;

    GFX.texture_cache[GFX.current_texture_index++] = gfx_add_to_palette(r, g, b, a);
  }

  *id = tidx;
}

gfxrgb gfx_argb32_to_gfxrgb (gfxuint color)
{
  gfxrgb rgb;
  float _255inv = 0.003921568627;

  rgb.r = (float)((color & 0x00ff0000) >> 16) * _255inv;
  rgb.g = (float)((color & 0x0000ff00) >>  8) * _255inv;
  rgb.b = (float) (color & 0x000000ff) * _255inv;

  return rgb;
}

gfxuint gfx_gfxrgb_to_argb32 (gfxrgb color)
{
  gfxuint a = (gfxuint)255 << 24;
  gfxuint r = (gfxuint)(color.r * 255) << 16;
  gfxuint g = (gfxuint)(color.g * 255) <<  8;
  gfxuint b = (gfxuint)(color.b * 255) <<  0;

  return (gfxuint)(a | r | g | b);
}

gfxrgb gfx_color_mix (gfxrgb c1, gfxrgb c2, float a)
{
  gfxrgb outc;

  outc.r = c1.r + ((c2.r - c1.r) * a);
  outc.g = c1.g + ((c2.g - c1.g) * a);
  outc.b = c1.b + ((c2.b - c1.b) * a);

  return outc;
}

gfxrgb gfx_color_blend (gfxrgb c1, gfxrgb c2)
{
  gfxrgb outc;

  outc.r = c1.r * c2.r;
  outc.g = c1.g * c2.g;
  outc.b = c1.b * c2.b;

  return outc;
}

gfxrgb gfx_color_add (gfxrgb c1, gfxrgb c2)
{
  gfxrgb outc;

  outc.r = gfx_min2(c1.r + c2.r, 1.0);
  outc.g = gfx_min2(c1.g + c2.g, 1.0);
  outc.b = gfx_min2(c1.b + c2.b, 1.0);

  return outc;
}


gfxrgb gfx_color_brightness (gfxrgb color, float brightness)
{
  gfxrgb outc;

  outc.r = color.r * brightness;
  outc.g = color.g * brightness;
  outc.b = color.b * brightness;

  return outc;
}

int gfx_is_backfacing (gfxv4 *v1, gfxv4 *v2, gfxv4 *v3)
{
  return ((v2->y - v1->y) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->y - v1->y)) < 0;
}

void gfx_lerp (gfxvert *out, gfxvert *v1, gfxvert *v2, float amt)
{
  out->transformed.x = v1->transformed.x + ((v2->transformed.x - v1->transformed.x) * amt);
  out->transformed.y = v1->transformed.y + ((v2->transformed.y - v1->transformed.y) * amt);
  out->transformed.z = v1->transformed.z + ((v2->transformed.z - v1->transformed.z) * amt);
  out->transformed.w = v1->transformed.w + ((v2->transformed.w - v1->transformed.w) * amt);

  out->normal.x = v1->normal.x + ((v2->normal.x - v1->normal.x) * amt);
  out->normal.y = v1->normal.y + ((v2->normal.y - v1->normal.y) * amt);
  out->normal.z = v1->normal.z + ((v2->normal.z - v1->normal.z) * amt);
  out->normal.w = v1->normal.w + ((v2->normal.w - v1->normal.w) * amt);

  out->uv.u = v1->uv.u + ((v2->uv.u - v1->uv.u) * amt);
  out->uv.v = v1->uv.v + ((v2->uv.v - v1->uv.v) * amt);
}

void gfx_use_context (struct _gfx *ctx)
{
  __GFX = ctx;
}

int gfx_init ()
{
  GFX.vertices = NULL;
  GFX.indices = NULL;
  GFX.uvs = NULL;
  GFX.colors = NULL;
  GFX.texture = NULL;

  GFX.enabled = 0;

  GFX.attr_type = GFX_ATTR_RGB;

  GFX.solid_color.r = 1.0;
  GFX.solid_color.g = 1.0;
  GFX.solid_color.b = 1.0;

  GFX.clear_color = 0;
  GFX.num_lights = 0;
  GFX.num_palette_colors = 0;

  GFX.draw_mode = GFX_FLAT_FILL_MODE;

  GFX.current_texture_index = 0;

  return 1;
}

void gfx_clear_color (float rf, float gf, float bf)
{
  int r = (int)(rf * 255);
  int g = (int)(gf * 255);
  int b = (int)(bf * 255);

  GFX.clear_color = (255 << 24 | r << 16 | g << 8 | b);
}

void gfx_clear ()
{
  int i, l = GFX.target_width * GFX.target_height;
  for (i = 0; i < l; i++) GFX.target[i] = GFX.clear_color;
  for (i = 0; i < l; i++) GFX.depth_buffer[i] = 0.0;
}

unsigned int gfx_memory_requirements ()
{
  return (gfxuint)sizeof(struct _gfx);
}

void gfx_matrix_mode (int m)
{
  switch (m) {
    case GFX_MODEL_MATRIX:
      GFX.active = &GFX.model;
    break;
    case GFX_VIEW_MATRIX:
      GFX.active = &GFX.view;
    break;
    case GFX_PROJECTION_MATRIX:
      GFX.active = &GFX.projection;
    break;
  }
}

void gfx_bind_render_target (gfxuint *target, int width, int height)
{
  GFX.target = target;
  GFX.target_width = width;
  GFX.target_height = height;
}

void gfx_bind_depth_buffer (float *depth_buffer)
{
  GFX.depth_buffer = depth_buffer;
}

void gfx_bind_arrays (float *vertices, int vsize, int *indices, int isize)
{
  GFX.vertices = vertices;
  GFX.indices = indices;

  GFX.vertex_count = vsize;
  GFX.index_count = isize;
}

void gfx_bind_attr (int attr, float *data)
{
  GFX.attr_type = attr;

  switch (attr) {
    case GFX_ATTR_RGB:
      GFX.solid_color.r = data[0];
      GFX.solid_color.g = data[1];
      GFX.solid_color.b = data[2];
      GFX.uvs = NULL;
      GFX.colors = NULL;
    break;
    case GFX_ATTR_COLORS:
      GFX.colors = data;
      GFX.uvs = NULL;
    break;
    case GFX_ATTR_UVS:
      GFX.uvs = data;
      GFX.colors = NULL;
    break;
  }
}

void gfx_bind_texture (int id)
{
  GFX.texture = &GFX.texture_cache[GFX.textures[id].index];
  GFX.texture_width = GFX.textures[id].width;
  GFX.texture_height = GFX.textures[id].height;
}

void gfx_rotate (float x, float y, float z, float a)
{
  gfxm4 rotation, transform;
  gfx_m4_rotation(&rotation, x, y, z, a);
  gfx_m4_mult(&transform, GFX.active, &rotation);
  gfx_m4_copy(GFX.active, &transform);
}

void gfx_translate (float x, float y, float z)
{
  gfxm4 translation, transform;
  gfx_m4_translation(&translation, x, y, z);
  gfx_m4_mult(&transform, GFX.active, &translation);
  gfx_m4_copy(GFX.active, &transform);
}

void gfx_scale (float x, float y, float z)
{
  gfxm4 scale, transform;
  gfx_m4_scale(&scale, x, y, z);
  gfx_m4_mult(&transform, GFX.active, &scale);
  gfx_m4_copy(GFX.active, &transform);
}

void gfx_perspective (float fov, float ar, float nearz, float farz)
{
  gfxm4 perspective, transform;
  gfx_m4_perspective(&perspective, ((fov * GFX_PI) / 180.0f), ar, nearz, farz);
  gfx_m4_mult(&transform, GFX.active, &perspective);
  gfx_m4_copy(GFX.active, &transform);
}

void gfx_identity ()
{
  gfx_m4_ident(GFX.active);
}

void gfx_draw_line (float x1, float y1, float x2, float y2, gfxuint color)
{
  int count = 0, max;
  int width = GFX.target_width;

  float xdiff = x2 - x1;
  float ydiff = y2 - y1;

  float xstep = xdiff == 0 ? 0 : xdiff < 0 ? -1 : 1;
  float ystep = ydiff == 0 ? 0 : ydiff < 0 ? -1 : 1;

  float axdiff = fabs(xdiff);
  float aydiff = fabs(ydiff);

  if (axdiff > aydiff) {
    ystep = ydiff / axdiff;
    max = axdiff;
  } else {
    xstep = xdiff / aydiff;
    max = aydiff;
  }

  do {
    /* temporary check until screen-space clipping */
    if (y1 >= 0 && y1 < GFX.target_height && x1 > 0 && x1 < width)
      GFX.target[(int)y1 * width + (int)x1] = color;

    x1 += xstep;
    y1 += ystep;
  } while (++count < max);
}

gfxrgb gfx_light_color(gfxrgb color, gfxnormal normal, gfxv4 point)
{
  int i;
  gfxrgb c, final_color;
  gfxv4 light;
  float distance, attenuation, brightness;

#if 0
  gfx_v4_init(&light, 0, 0, 1, 0);
  brightness = gfx_v4_dotp(&light, &normal);
  brightness = brightness < 0 ? -brightness : brightness;

  if (brightness < 0.2) {
    brightness = 0.2; /* ambient light */
  }

  final_color = gfx_color_brightness(color, brightness);
#else
  final_color.r = final_color.g = final_color.b = 0;
#endif

  for (i = 0; i < GFX.num_lights; i++) {
    if (GFX.lights[i].type == GFX_POINT_LIGHT) {
      gfx_v4_mult(&light, &GFX.lights[i].position, &GFX.view);
      gfx_v4_sub(&light, &point, &light);
      distance = gfx_v4_length(&light);
      gfx_v4_normalize(&light, &light);
      attenuation = 1.0 / (1.0 + 0.0001 * pow(distance, 2));
      brightness = gfx_v4_dotp(&normal, &light);
      brightness = brightness < 0 ? -brightness : 0;
      c = gfx_color_brightness(GFX.lights[i].color, brightness * attenuation * GFX.lights[i].intensity);
      c = gfx_color_blend(color, c);
      final_color = gfx_color_add(final_color, c);
    }
  }

  return final_color;
}

void gfx_point_light (float x, float y, float z, float r, float g, float b, float intensity)
{
  if (GFX.num_lights < 8) {
    int i = GFX.num_lights++;

    GFX.lights[i].position.x = x;
    GFX.lights[i].position.y = y;
    GFX.lights[i].position.z = z;
    GFX.lights[i].position.w = 1;

    GFX.lights[i].color.r = r;
    GFX.lights[i].color.g = g;
    GFX.lights[i].color.b = b;

    GFX.lights[i].intensity = intensity;
    GFX.lights[i].type = GFX_POINT_LIGHT;
  }
}

void gfx_directional_light (float x, float y, float z, float r, float g, float b, float intensity)
{
  if (GFX.num_lights < 8) {
    int i = GFX.num_lights++;

    GFX.lights[i].position.x = x;
    GFX.lights[i].position.y = y;
    GFX.lights[i].position.z = z;
    GFX.lights[i].position.w = 1;

    GFX.lights[i].color.r = r;
    GFX.lights[i].color.g = g;
    GFX.lights[i].color.b = b;

    GFX.lights[i].intensity = intensity;
    GFX.lights[i].type = GFX_DIRECTIONAL_LIGHT;
  }
}

void gfx_reset_lights ()
{
  GFX.num_lights = 0;
}

void gfx_draw_mode (int mode)
{
  GFX.draw_mode = mode;
}

void gfx_clip_flags (int id)
{
  gfxv4 *camera = &GFX.vertex_pipe[id].transformed;

  float y = camera->y;
  float x = camera->x;
  float z = camera->z;
  float w = camera->w;

  GFX.vertex_pipe[id].clip_flags = (gfxbyte)(z < -w)
    | ((gfxbyte)(z >  w) << 1)
    | ((gfxbyte)(x < -w) << 2)
    | ((gfxbyte)(x >  w) << 3)
    | ((gfxbyte)(y < -w) << 4)
    | ((gfxbyte)(y >  w) << 5);
}

void gfx_init_edge (
  gfxedge* e, gfxv2* vert1, gfxv2* vert2, 
  float u1, float u2, float v1, float v2, float z1, float z2
)
{
  float yd = vert2->y - vert1->y;
  float xd = vert2->x - vert1->x;
  float ud = u2 - u1;
  float vd = v2 - v1;
  float zd = z2 - z1;
  float prestep, fxstep, fzstep, fustep, fvstep;
  float ydinv = 1.0 / yd;

  e->y_start = (int)ceil(vert1->y);
  e->y_end = (int)ceil(vert2->y);

  prestep = ((float)e->y_start - vert1->y);

  fxstep = xd * ydinv;
  fzstep = zd * ydinv;
  fustep = ud * ydinv;
  fvstep = vd * ydinv;

  e->x = vert1->x + prestep * fxstep;

  e->z = z1 + prestep * fzstep;
  e->u = u1 + prestep * fustep;
  e->v = v1 + prestep * fvstep;

  e->x_step = fxstep;
  e->z_step = fzstep;
  e->u_step = fustep;
  e->v_step = fvstep;
}

void gfx_draw_span_flat (gfxedge *e1, gfxedge *e2, int y, gfxuint color)
{
  int sx1, sx2;
  unsigned int *dst, *max, offs;
  float *zbuf, z, zstep;

  float x1 = e1->x;
  float x2 = e2->x;
  float z1 = e1->z;
  float z2 = e2->z;

  sx1 = (int)floor(x1);
  sx2 = (int)floor(x2);

  z = z1;
  zstep = (z2 - z1) / (x2 - x1);

  if (sx2 > GFX.target_width) sx2 = GFX.target_width;
  if (sx1 < 0) {
    z += (zstep * -(sx1));
    sx1 = 0;
  }

  offs = GFX.target_width * y;
  dst = GFX.target + offs + sx1;
  max = GFX.target + offs + sx2;
  zbuf = GFX.depth_buffer + offs + sx1;

  while (dst < max) {
    if (z > *zbuf) {
      *dst = color;
      *zbuf = z;
    }

    dst++;
    zbuf++;
    z += zstep;
  }
}

void gfx_draw_span_textured (gfxedge *e1, gfxedge *e2, int y)
{
  int sx1, sx2;
  unsigned int *dst, *max, offs;
  float *zbuf, z, u, v;
  float zstep, ustep, vstep;
  float xdinv;
  // float uleft, vleft, uright, vright, zleft, zright;
  // float ustepaff, vstepaff;
  // int subdivs, remaining;

  float x1 = e1->x;
  float x2 = e2->x;
  float z1 = e1->z;
  float z2 = e2->z;
  float u1 = e1->u;
  float u2 = e2->u;
  float v1 = e1->v;
  float v2 = e2->v;

  int tw = GFX.texture_width;
  int th = GFX.texture_height;
  int tu, tv;

  sx1 = (int)floor(x1);
  sx2 = (int)floor(x2);

  u = u1;
  v = v1;
  z = z1;

  // uleft = u1;
  // vleft = v1;
  // zleft = z1;

  xdinv = 1.0 / (x2 - x1);
  zstep = (z2 - z1) * xdinv;
  ustep = (u2 - u1) * xdinv;
  vstep = (v2 - v1) * xdinv;

  if (sx2 > GFX.target_width) sx2 = GFX.target_width;
  if (sx1 < 0) {
    z += (zstep * -(sx1));
    u += (ustep * -(sx1));
    v += (vstep * -(sx1));
    sx1 = 0;
  }

  offs = GFX.target_width * y;
  dst = GFX.target + offs + sx1;
  max = GFX.target + offs + sx2;
  zbuf = GFX.depth_buffer + offs + sx1;

  // subdivs = (sx2 - sx1) / 16;
  // remaining = (sx2 - sx1) % 16;

  // while (subdivs--) {
  //   float zleftinv = 1.0 / zleft;
  //   float zrightinv = 1.0 / zright;
  // }

  // while (remaining--) {

  // }

  while (dst < max) {
    if (z > *zbuf) {
      float zinv = 1.0 / z;
      int index;
      tu = (int)floor(u * zinv * tw);
      tv = (int)floor(v * zinv * th);
      index = tv * GFX.texture_width + tu;
      if (GFX.texture[index] != 0) {
        *dst = GFX.palette_argb32[GFX.texture[index]];
        *zbuf = z;
      }
    }

    dst++;
    zbuf++;

    z += zstep;
    u += ustep;
    v += vstep;
  }
}

void gfx_scan_edges (gfxedge *e1, gfxedge *e2, int left, gfxuint color)
{
  gfxedge *tmp;
  int y1, y2, i;

  y1 = e2->y_start;
  y2 = e2->y_end;

  if (left) { tmp = e1; e1 = e2; e2 = tmp; }

  if (GFX.attr_type == GFX_ATTR_UVS) {
    for (i = y1; i < y2; i++) {
      if (i < 0 || i >= GFX.target_height)
        continue;

      gfx_draw_span_textured(e1, e2, i);
      
      e1->x += e1->x_step;
      e1->z += e1->z_step;
      e1->u += e1->u_step;
      e1->v += e1->v_step;

      e2->x += e2->x_step;
      e2->z += e2->z_step;
      e2->u += e2->u_step;
      e2->v += e2->v_step;
    }
  } else {
    for (i = y1; i < y2; i++) {
      if (i < 0 || i >= GFX.target_height)
        continue;

      gfx_draw_span_flat(e1, e2, i, color);
      
      e1->x += e1->x_step;
      e1->z += e1->z_step;

      e2->x += e2->x_step;
      e2->z += e2->z_step;
    }
  }
}

void gfx_draw_triangle (int index)
{
  gfxpoly *tri = &GFX.visible[index];
  gfxv2 *vert1, *vert2, *vert3, *tmp;
  gfxedge e1, e2, e3;
  float area;
  float w1, w2, w3, tv;
  gfxuint color = 0;
  float u1, v1, u2, v2, u3, v3;
  int i1, i2, i3;

  i1 = tri->verts[0];
  i2 = tri->verts[1];
  i3 = tri->verts[2];

  vert1 = &tri->projected[0];
  vert2 = &tri->projected[1];
  vert3 = &tri->projected[2];

  w1 = 1.0 / GFX.vertex_pipe[i1].transformed.w;
  w2 = 1.0 / GFX.vertex_pipe[i2].transformed.w;
  w3 = 1.0 / GFX.vertex_pipe[i3].transformed.w;

  if (GFX.attr_type == GFX_ATTR_UVS) {
    int i, j, max = GFX.num_palette_colors;

    u1 = GFX.vertex_pipe[i1].uv.u * w1;
    v1 = GFX.vertex_pipe[i1].uv.v * w1;
    u2 = GFX.vertex_pipe[i2].uv.u * w2;
    v2 = GFX.vertex_pipe[i2].uv.v * w2;
    u3 = GFX.vertex_pipe[i3].uv.u * w3;
    v3 = GFX.vertex_pipe[i3].uv.v * w3;

    if (GFX.enabled & GFX_LIGHTS) {
      for (i = 0, j = 0; i < max; i++) {
        gfxrgb c = gfx_light_color(GFX.palette[i], tri->normal, tri->center);
        GFX.palette_argb32[j++] = gfx_gfxrgb_to_argb32(c);
      }
    } else {
      for (i = 0, j = 0; i < max; i++) {
        GFX.palette_argb32[j++] = gfx_gfxrgb_to_argb32(GFX.palette[i]);
      }
    }
  } else if (GFX.attr_type == GFX_ATTR_COLORS) {
    if (GFX.enabled & GFX_LIGHTS) {
      gfxrgb c = gfx_light_color(tri->color, tri->normal, tri->center);
      color = gfx_gfxrgb_to_argb32(c);
    } else {
      color = gfx_gfxrgb_to_argb32(tri->color);
    }
  } else {
    if (GFX.enabled & GFX_LIGHTS) {
      gfxrgb c = gfx_light_color(GFX.solid_color, tri->normal, tri->center);
      color = gfx_gfxrgb_to_argb32(c);
    } else {
      color = gfx_gfxrgb_to_argb32(GFX.solid_color);
    }
  }

  if (vert2->y < vert1->y) {
    tmp = vert1; vert1 = vert2; vert2 = tmp;
    tv = w1; w1 = w2; w2 = tv;
    tv = u1; u1 = u2; u2 = tv;
    tv = v1; v1 = v2; v2 = tv;
  }

  if (vert3->y < vert2->y) {
    tmp = vert2; vert2 = vert3; vert3 = tmp;
    tv = w2; w2 = w3; w3 = tv;
    tv = u2; u2 = u3; u3 = tv;
    tv = v2; v2 = v3; v3 = tv;
  }

  if (vert2->y < vert1->y) {
    tmp = vert1; vert1 = vert2; vert2 = tmp;
    tv = w1; w1 = w2; w2 = tv;
    tv = u1; u1 = u2; u2 = tv;
    tv = v1; v1 = v2; v2 = tv;
  }

  area = gfx_v2_area(vert1, vert3, vert2);

  if (area == 0) {
    return;
  }

  gfx_init_edge(&e1, vert1, vert3, u1, u3, v1, v3, w1, w3);
  gfx_init_edge(&e2, vert1, vert2, u1, u2, v1, v2, w1, w2);
  gfx_init_edge(&e3, vert2, vert3, u2, u3, v2, v3, w2, w3);

  gfx_scan_edges(&e1, &e2, area > 0, color);
  gfx_scan_edges(&e1, &e3, area > 0, color);
}

void gfx_project_to_screen (gfxv2 *out, gfxvert *v)
{
  float inv = 1.0 / v->transformed.w;

  out->x = (v->transformed.x * inv + 1.0) * ((float)GFX.target_width * 0.5);
  out->y = (v->transformed.y * inv + 1.0) * ((float)GFX.target_height * 0.5);
  out->y = GFX.target_height - 1 - out->y;
}

void gfx_add_visible_indexed (int i, int v1, int v2, int v3, int c, gfxnormal *normal, gfxv4 *center)
{
  GFX.visible[i].verts[0] = v1;
  GFX.visible[i].verts[1] = v2;
  GFX.visible[i].verts[2] = v3;

  GFX.visible[i].normal = *normal;
  GFX.visible[i].center = *center;

  if (GFX.attr_type == GFX_ATTR_COLORS) {
    GFX.visible[i].color.r = GFX.colors[c+0];
    GFX.visible[i].color.g = GFX.colors[c+1];
    GFX.visible[i].color.b = GFX.colors[c+2];
  }
}

void gfx_clip_low (int *verts, int *vidx, int axis, int *cnt)
{
  int outv[9], _i = 0, i = 0;
  int count = *cnt;
  float amt;
  int curr_inside, prev_inside;
  gfxvert vert, *prev_vert, *curr_vert;

  curr_vert = &GFX.vertex_pipe[verts[count - 1]];
  curr_inside = -(((float *)(&curr_vert->transformed))[axis]) <= curr_vert->transformed.w;

  for (i = 0; i < count; i++) {
    prev_vert = curr_vert;
    curr_vert = &GFX.vertex_pipe[verts[i]];

    prev_inside = curr_inside;
    curr_inside = -(((float *)(&curr_vert->transformed))[axis]) <= curr_vert->transformed.w;

    if (curr_inside ^ prev_inside) {
      float pw = prev_vert->transformed.w;
      float cw = curr_vert->transformed.w;
      float pv = ((float *)(&prev_vert->transformed))[axis];
      float cv = ((float *)(&curr_vert->transformed))[axis];

      amt = (pw + pv) / ((pw + pv) - (cw + cv));

      gfx_lerp(&vert, prev_vert, curr_vert, amt);
      GFX.vertex_pipe[*vidx] = vert;
      outv[_i++] = (*vidx)++;
    }

    if (curr_inside) {
      outv[_i++] = verts[i];
    }
  }

  for (i = 0; i < _i; i++) {
    verts[i] = outv[i];
  }

  *cnt = _i;
}

void gfx_clip_high (int *verts, int *vidx, int axis, int *cnt)
{
  int outv[9], _i = 0, i = 0;
  int count = *cnt;
  float amt;
  int curr_inside, prev_inside;
  gfxvert vert, *prev_vert, *curr_vert;

  curr_vert = &GFX.vertex_pipe[verts[count - 1]];
  curr_inside = ((float *)(&curr_vert->transformed))[axis] < curr_vert->transformed.w;

  for (i = 0; i < count; i++) {
    prev_vert = curr_vert;
    curr_vert = &GFX.vertex_pipe[verts[i]];

    prev_inside = curr_inside;
    curr_inside = ((float *)(&curr_vert->transformed))[axis] < curr_vert->transformed.w;

    if (curr_inside ^ prev_inside) {
      float pw = prev_vert->transformed.w;
      float cw = curr_vert->transformed.w;
      float pv = ((float *)(&prev_vert->transformed))[axis];
      float cv = ((float *)(&curr_vert->transformed))[axis];

      amt = (pw - pv) / ((pw - pv) - (cw - cv));

      gfx_lerp(&vert, prev_vert, curr_vert, amt);
      GFX.vertex_pipe[*vidx] = vert;
      outv[_i++] = (*vidx)++;
    }

    if (curr_inside) {
      outv[_i++] = verts[i];
    }
  }

  for (i = 0; i < _i; i++) {
    verts[i] = outv[i];
  }

  *cnt = _i;
}

void gfx_draw_arrays (int start, int end)
{
  gfxvert *pv1, *pv2, *pv3;
  gfxm4 mv, mvp;
  gfxv4 tmp, tmp1, tmp2;
  int i, vidx, pidx, vsize, isize, uvi;
  gfxvert *pipe = (gfxvert *)&GFX.vertex_pipe;
  gfxnormal normal;
  gfxv4 center;

  gfx_m4_mult(&mv, &GFX.view, &GFX.model);
  gfx_m4_mult(&mvp, &GFX.projection, &mv);

  vsize = GFX.vertex_count * 3;
  isize = end == -1 ? GFX.index_count * 3 : (end + 1) * 3;
  uvi = 0;
  vidx = 0;
  pidx = 0;

  for (i = 0; i < vsize; i+=3, uvi+=2) {
    gfx_v4_init(&tmp, GFX.vertices[i], GFX.vertices[i+1], GFX.vertices[i+2], 1);
    gfx_v4_mult(&pipe[vidx].transformed, &tmp, &mvp);
    gfx_v4_mult(&pipe[vidx].screen_space, &tmp, &mv); /* used for calculating face normals */

    if (GFX.attr_type == GFX_ATTR_UVS) {
      pipe[vidx].uv.u = GFX.uvs[uvi];
      pipe[vidx].uv.v = GFX.uvs[uvi+1];
    }

    gfx_clip_flags(vidx++);
  }

  for (i = start; i < isize; i+=3) {
    int i1 = GFX.indices[i+0];
    int i2 = GFX.indices[i+1];
    int i3 = GFX.indices[i+2];
    int flags;

    pv1 = &pipe[i1];
    pv2 = &pipe[i2];
    pv3 = &pipe[i3];

    /* fast frustum check */
    if (pv1->clip_flags & pv2->clip_flags & pv3->clip_flags) {
      continue;
    }

    if (gfx_is_backfacing(&pv1->screen_space, &pv2->screen_space, &pv3->screen_space)) {
      continue;
    }

    flags = pv1->clip_flags | pv2->clip_flags | pv3->clip_flags;

    gfx_v4_sub(&tmp1, &pv2->screen_space, &pv1->screen_space);
    gfx_v4_sub(&tmp2, &pv3->screen_space, &pv1->screen_space);
    gfx_v4_crossp(&normal, &tmp1, &tmp2);
    gfx_v4_normalize(&normal, &normal);

    center.x = (pv1->screen_space.x + pv2->screen_space.x + pv3->screen_space.x) * 0.3333;
    center.y = (pv1->screen_space.y + pv2->screen_space.y + pv3->screen_space.y) * 0.3333;
    center.z = (pv1->screen_space.z + pv2->screen_space.z + pv3->screen_space.z) * 0.3333;

    if (flags == 0) {
      gfx_add_visible_indexed(pidx++, i1, i2, i3, i, &normal, &center);
    } else {
      int verts[9], count, j;

      verts[0] = i1;
      verts[1] = i2;
      verts[2] = i3;

      count = 3;

      if (flags &  1) gfx_clip_low(verts, &vidx, 2, &count);
      if (flags &  2) gfx_clip_high(verts, &vidx, 2, &count);
      if (flags &  4) gfx_clip_low(verts, &vidx, 0, &count);
      if (flags &  8) gfx_clip_high(verts, &vidx, 0, &count);
      if (flags & 16) gfx_clip_low(verts, &vidx, 1, &count);
      if (flags & 32) gfx_clip_high(verts, &vidx, 1, &count);

      for (j = 1; j < count-1; j++) {
        gfx_add_visible_indexed(pidx++, verts[0], verts[j], verts[j+1], i, &normal, &center);
      }
    }
  }

  for (i = 0; i < pidx; i++) {
    gfx_project_to_screen(&GFX.visible[i].projected[0], &pipe[GFX.visible[i].verts[0]]);
    gfx_project_to_screen(&GFX.visible[i].projected[1], &pipe[GFX.visible[i].verts[1]]);
    gfx_project_to_screen(&GFX.visible[i].projected[2], &pipe[GFX.visible[i].verts[2]]);

    gfx_draw_triangle(i);
  }
}

void gfx_draw_textured_quad (int texture_id)
{
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
    0.0, 1.0,
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0
  };

  gfx_bind_arrays(vertices, 4, indices, 2);
  gfx_bind_attr(GFX_ATTR_UVS, uvs);
  gfx_bind_texture(texture_id);
  gfx_draw_arrays(0, -1);
}

void gfx_draw_text_8x8 (char font[][8], const char *str, int length, int offx, int offy)
{
  int i, y, x, l, min = 0, max = length;
  gfxuint c;
  gfxuint *buf = GFX.target;
  int w = GFX.target_width;
  int h = GFX.target_height;

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
