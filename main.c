#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define IMPL_COLOR_H
#define IMPL_MATRIX_H
#define IMPL_RASTERIZE_H

#include "src/color.h"
#include "src/matrix.h"
#include "src/rasterize.h"

/// point lerp
point_t plerp(point_t p1, point_t p2, float u) {
  return (point_t) {
    (1-u) * p1.x + u * p2.x,
    (1-u) * p1.y + u * p2.y
  };
}

void draw_vector(image_t image, point_t start, point_t vec) {
  point_t end = {roundf(start.x + vec.x), roundf(start.y + vec.y)};
  draw_line(image, start, end, WHITE);
  draw_cross(image, end.x, end.y, RED);
}

void draw_bezier(image_t image, point_t p1, point_t p2, point_t p3, point_t p4) {
  draw_cross(image, p1.x, p1.y, RED);
  draw_cross(image, p2.x, p2.y, RED);
  draw_cross(image, p3.x, p3.y, RED);
  draw_cross(image, p4.x, p4.y, RED);

  for (float u = 0; u <= 1; u += 1.f/256) {
    // de casteljau's algorithm
    point_t a1 = plerp(p1, p2, u);
    point_t a2 = plerp(p2, p3, u);
    point_t a3 = plerp(p3, p4, u);

    point_t b1 = plerp(a1, a2, u);
    point_t b2 = plerp(a2, a3, u);

    point_t c1 = plerp(b1, b2, u);
    set_pixel(image, nearbyintf(c1.x), nearbyintf(c1.y), WHITE);
  }
}

void draw_hermite(image_t image, point_t p1, point_t t1, point_t p2, point_t t2) {
  draw_vector(image, p1, t1);
  draw_vector(image, p2, t2);

  point_t prev = p1;
  for (double u = 1.f/128; u <= 1; u += 1.f/128) {
    double h0 = (1-u) * (1-u) * (1+2*u);
    double h1 = 10 * u * (1-u) * (1-u);
    double h2 = 10 * -(u*u) * (1-u);
    double h3 = (3 - 2*u) * u * u;

    point_t next = {nearbyint(h0*p1.x + h1*t1.x + h2*t2.x + h3*p2.x),
                    nearbyint(h0*p1.y + h1*t1.y + h2*t2.y + h3*p2.y)};
    draw_line(image, prev, next, GREEN);
    prev = next;
  }
}

long double norm(double complex n) {
  return creall(n) * creall(n) + cimagl(n) * cimagl(n);
}

uint8_t diverges(long double complex c, int iterations) {
  long double complex z = 0;
  for (int i = 1; i <= iterations; i++) {
    z = z * z + c;
    if (norm(z) > 4) {
      return i;
    }
  }
  return 0;
}

void mandelbrot(image_t image) {
  int iterations = 64;
  double aspect_ratio = ((double)image.height) / image.width;
  for (int y = 0; y < image.height; y++) {
    for (int x = 0; x < image.width; x++) {
      long double complex c = (3. *  x / image.width - 2) +
                              (aspect_ratio * (3. * y / image.height - 1.5)) * I;
      uint8_t d;
      if ((d = diverges(c, iterations))) {
        float u = d / (float)iterations;
        u = pow(u, 0.4); // gamma correction
        rgb_t color = rgb_bezier_quadratic(BLACK, BLUE, WHITE, u);
        set_pixel(image, x, y, color);
      }
      // else{
      //   set_pixel(image, x, y, BLUE);
      // }
    }
  }
}

void linear_gradient(image_t image, rgb_t col1, rgb_t col2) {
  for (int i = 0; i < image.width; i++) {
    float t = (float)i / (image.width - 1);
    rgb_t c = rgb_lerp(col1, col2, t);
    for (int j = 0; j < image.height; j++) {
      set_pixel(image, i, j, c);
    }
  }
}

void bezier_gradient(image_t image, rgb_t col1, rgb_t col2, rgb_t col3) {

  for (int i = 0; i < image.width; i++) {
    float t = (float)i / (image.width - 1);
    rgb_t c = rgb_bezier_quadratic(col1, col2, col3, t);
    for (int j = 0; j < image.height; j++) {
      set_pixel(image, i, j, c);
    }
  }
}

void save_to_ppm(image_t image, char* filename) {
  FILE* f = fopen(filename, "wb");

  fprintf(f, "P6\n");  // magic number for ppm format
  fprintf(f, "%d %d\n", image.width, image.height);
  fprintf(f, "255\n");  // max color value

  for (int y = 0; y < image.height; y++) {
    for (int x = 0; x < image.width; x++) {
      rgb_t p = get_pixel(image, x, y);
      fputc((uint8_t)nearbyintf(p.r * 255), f);
      fputc((uint8_t)nearbyintf(p.g * 255), f);
      fputc((uint8_t)nearbyintf(p.b * 255), f);
    }
  }

  fclose(f);
}

int main(int argc, char *argv[]) {
  char* filename = "hsv_bar.ppm";
  // int scale = 800;
  // int width = 4 * scale;
  // int height = 3 * scale;

  int width = 200;
  int height = 30;

  rgb_t* pixels = (rgb_t*)malloc(sizeof(rgb_t) * width * height);
  image_t image = {width, height, pixels};
  set_uniform(image, BLACK);
  // mandelbrot(image);

  hsv_t col = (hsv_t){ 0, 1, 1 };
  for (int i = 0; i < width; i++) {
    col.h = ((float) i) / width;
    rgb_t col_rgb = hsv2rgb(col);
    for (int j = 0; j < height; j++) {
      set_pixel(image, i, j, col_rgb);
    }
  }

  save_to_ppm(image, filename);
  free(image.pixels);
  printf("Written %s\n", filename);

  return 0;
}
