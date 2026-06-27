#ifndef RASTERIZE_H
#define RASTERIZE_H

#include <stdbool.h>
#include <stdlib.h>

#include "color.h"

typedef struct {
  int width;
  int height;
  rgb_t* pixels;
} image_t;

typedef struct {
  float x;
  float y;
} point_t;

void set_pixel(image_t image, int x, int y, rgb_t pixel);
rgb_t get_pixel(image_t image, size_t x, size_t y);

void set_uniform(image_t image, rgb_t color);

void draw_cross(image_t image, int x, int y, rgb_t color);
void draw_line(image_t image, point_t p1, point_t p2, rgb_t color);

#endif /* #ifndef RASTERIZE_H */

#ifdef IMPL_RASTERIZE_H

void set_pixel(image_t image, int x, int y, rgb_t pixel) {
  if (x < 0 || y < 0 || x >= image.width || y >= image.height) return;
  image.pixels[image.width * y + x] = pixel;
}

rgb_t get_pixel(image_t image, size_t x, size_t y) {
  return image.pixels[image.width * y + x];
}

void set_uniform(image_t image, rgb_t color) {
  for (int y = 0; y < image.height; y++) {
    for (int x = 0; x < image.width; x++) {
      set_pixel(image, x, y, color);
    }
  }
}

void draw_cross(image_t image, int x, int y, rgb_t color) {
  set_pixel(image, x, y-2, color);
  set_pixel(image, x, y-1, color);
  set_pixel(image, x, y,   color);
  set_pixel(image, x, y+1, color);
  set_pixel(image, x, y+2, color);
  set_pixel(image, x-2, y, color);
  set_pixel(image, x-1, y, color);
  set_pixel(image, x+1, y, color);
  set_pixel(image, x+2, y, color);
}

point_t mirror_on_axes(point_t p, bool mirror_x, bool mirror_y, bool flip_xy) {
  if (mirror_x) {
    p.x *= -1;
  }
  if (mirror_y) {
    p.y *= -1;
  }
  if (flip_xy) {
    float tmp = p.x;
    p.x = p.y;
    p.y = tmp;
  }
  return p;
}

void draw_line(image_t image, point_t p1, point_t p2, rgb_t color) {
  // bresenham's algorithm
  int dx = p2.x - p1.x;
  int dy = p2.y - p1.y;
  // draw line as if dx > 0, dy > 0, dx >= dy (0 <= slope <= 1)
  // mirror if needed
  bool flip_x = dx < 0;
  bool flip_y = dy < 0;

  int adx = abs(dx);
  int ady = abs(dy);
  bool flip_xy = ady > adx;

  int da = fmax(adx, ady);
  int db = fmin(adx, ady);

  int e = 2 * db - da;
  point_t offset = {0, 0};
  set_pixel(image, p1.x, p1.y, color);
  while (offset.x < da || offset.y < db) {
    if (e <= 0) {
      offset.x++;
      e += 2 * db;
    }
    else {
      offset.x++;
      offset.y++;
      e += 2 * db - 2 * da;
    }
    point_t true_offset = mirror_on_axes(offset, flip_x, flip_y, flip_xy);
    set_pixel(image, p1.x + true_offset.x, p1.y + true_offset.y, color);
  }
}


#endif /* #ifdef IMPL_RASTERIZE_H */
