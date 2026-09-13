#ifndef RASTERIZE_H
#define RASTERIZE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "maths.h"
#include "matrix.h"

typedef struct {
   int width;
   int height;
   rgb_t *pixels;
} image_t;

typedef struct {
   float x;
   float y;
} point_t;

typedef struct {
   point_t top_left;
   point_t btm_right;
} bounding_box_t;

typedef struct {
   int width;
   int height;
   unsigned char* alpha;
   int codepoint;
} letter_t;


void set_pixel(image_t image, int x, int y, rgb_t pixel);
rgb_t get_pixel(image_t image, size_t x, size_t y);

void set_uniform(image_t image, rgb_t color);

void draw_cross(image_t image, int x, int y, rgb_t color);
void draw_line(image_t image, point_t p1, point_t p2, rgb_t color);

/** Draws triangle by calculating barycentric coordinates for every pixel.
 * Slow as hell and slightly inaccurate because of numerical instabilities
 * everywhere
 */
void draw_triangle_badly(image_t image, point_t p1, point_t p2, point_t p3,
                         rgb_t color);

// ------- Text -------

letter_t* load_bs(char* file);
void draw_letter(image_t image, int x, int y, letter_t letter, rgb_t color, int scale);
void draw_text(image_t image, int x, int y, letter_t* letters, char* text, rgb_t color, int scale);

#endif /* #ifndef RASTERIZE_H */

#ifdef IMPL_RASTERIZE_H

#ifndef IMPL_RASTERIZE_H_DONE
#define IMPL_RASTERIZE_H_DONE

void set_pixel(image_t image, int x, int y, rgb_t pixel) {
   if (x < 0 || y < 0 || x >= image.width || y >= image.height) return;
   image.pixels[image.width * y + x] = pixel;
}

rgb_t get_pixel(image_t image, size_t x, size_t y) {
   return image.pixels[image.width * y + x];
}

void set_uniform(image_t image, rgb_t color) {
   for (int y = 0; y < image.height; y++) {
      for (int x = 0; x < image.width; x++) { set_pixel(image, x, y, color); }
   }
}

void draw_cross(image_t image, int x, int y, rgb_t color) {
   set_pixel(image, x, y - 2, color);
   set_pixel(image, x, y - 1, color);
   set_pixel(image, x, y, color);
   set_pixel(image, x, y + 1, color);
   set_pixel(image, x, y + 2, color);
   set_pixel(image, x - 2, y, color);
   set_pixel(image, x - 1, y, color);
   set_pixel(image, x + 1, y, color);
   set_pixel(image, x + 2, y, color);
}

point_t mirror_on_axes(point_t p, bool mirror_x, bool mirror_y, bool flip_xy) {
   if (mirror_x) { p.x *= -1; }
   if (mirror_y) { p.y *= -1; }
   if (flip_xy) {
      float tmp = p.x;
      p.x       = p.y;
      p.y       = tmp;
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

   int adx      = abs(dx);
   int ady      = abs(dy);
   bool flip_xy = ady > adx;

   int da = max(adx, ady);
   int db = min(adx, ady);

   int e          = 2 * db - da;
   point_t offset = {0, 0};
   set_pixel(image, p1.x, p1.y, color);
   while (offset.x < da || offset.y < db) {
      if (e <= 0) {
         offset.x++;
         e += 2 * db;
      } else {
         offset.x++;
         offset.y++;
         e += 2 * db - 2 * da;
      }
      point_t true_offset = mirror_on_axes(offset, flip_x, flip_y, flip_xy);
      set_pixel(image, p1.x + true_offset.x, p1.y + true_offset.y, color);
   }
}

static bounding_box_t calc_bounding_box(point_t *points, size_t n_points) {
   bounding_box_t box = (bounding_box_t){(point_t){inf, inf},
                                         (point_t){-inf, -inf}};
   for (size_t i = 0; i < n_points; i++) {
      box.top_left.x  = min(box.top_left.x, points[i].x);
      box.top_left.y  = min(box.top_left.y, points[i].y);
      box.btm_right.x = max(box.btm_right.x, points[i].x);
      box.btm_right.y = max(box.btm_right.y, points[i].y);
   }
   return box;
}

void draw_triangle_badly(image_t image, point_t p1, point_t p2, point_t p3,
                         rgb_t color) {
   point_t pt_arr[3] = {p1, p2, p3};
   bounding_box_t bb = calc_bounding_box(pt_arr, 3);
   matrix_t r =
       mat_init(3, 3, (float[9]){p1.x, p2.x, p3.x, p1.y, p2.y, p3.y, 1, 1, 1},
                false); // point_mat

   matrix_t q   = mat_qr(r, true);
   matrix_t q_t = mat_transpose(q);

   for (int x = max(0, floor(bb.top_left.x));
        x <= min(image.width, ceil(bb.btm_right.x)); x++) {
      for (int y = max(0, floor(bb.top_left.y));
           y <= min(image.height, ceil(bb.btm_right.y)); y++) {
         matrix_t point_vec = mat_init(3, 1, (float[3]){x, y, 1}, false);
         matrix_t lhs       = mat_mul(q_t, point_vec);
         matrix_t barycentric_coords = back_substitute(r, lhs);
         // rgb_t p_color = {
         //     fminf(fmaxf(barycentric_coords.data[0], 0), 1),
         //     fminf(fmaxf(barycentric_coords.data[1], 0), 1),
         //     fminf(fmaxf(barycentric_coords.data[2], 0), 1)
         // };
         // set_pixel(image, x, y, p_color);
         if (mat_val(barycentric_coords, 0, 0) >= -1e-5 &&
             mat_val(barycentric_coords, 1, 0) >= -1e-5 &&
             mat_val(barycentric_coords, 2, 0) >= -1e-5) {
            set_pixel(image, x, y, color);
         }
         mat_free(lhs);
         mat_free(barycentric_coords);
      }
   }

   mat_free(q);
   mat_free(q_t);
}

unsigned int get_codepoint_from_utf8(char* symbol) {
   int c;
   switch(symbol[0] & 0b11110000) {
   case 0b11110000:
      c = symbol[0] & 0b00000111;
      c <<= 6;
      c |= symbol[1] & 0b00111111;
      c <<= 6;
      c |= symbol[2] & 0b00111111;
      c <<= 6;
      c |= symbol[3] & 0b00111111;
      break;
   case 0b11100000:
      c = symbol[0] & 0b00001111;
      c <<= 6;
      c |= symbol[1] & 0b00111111;
      c <<= 6;
      c |= symbol[2] & 0b00111111;
      break;
   case 0b11000000:
      c = symbol[0] & 0b00011111;
      c <<= 6;
      c |= symbol[1] & 0b00111111;
      break;
   default:
      c = symbol[0];
   }
   return c;
}

letter_t* load_bs(char* file) {
   FILE* fp;
   char* line = NULL;
   size_t len = 0;
   ssize_t read;

   fp = fopen(file, "r");
   if (fp == NULL) {
      perror("fopen");
      exit(EXIT_FAILURE);
   }

   read = getline(&line, &len, fp);
   if (strncmp(line, "BitSchrift", 10) != 0) {
      perror("keine gültige .bs-Datei :(");
      exit(EXIT_FAILURE);
   }

   getline(&line, &len, fp);
   letter_t* letters = calloc('~' - ' ' + 1, sizeof(letter_t));
   while ((read = getline(&line, &len, fp)) > 1) {
      char symbol[read];
      strlcpy(symbol, line, read);
      int codepoint = get_codepoint_from_utf8(symbol);
      int width = getline(&line, &len, fp) - 1;
      int height = 0;
      int capacity = width * 5;
      unsigned char* alpha = malloc(capacity);
      int i = 0;
      do {
         if (i == capacity) {
            capacity += width * 5;
            alpha = realloc(alpha, capacity);
         }
         for (int j = 0; j < width; j++, i++) {
            alpha[i] = line[j] == '0' ? 0 : 255;
         }
         height++;
      } while ((read = getline(&line, &len, fp)) > 1);

      if (codepoint > 127) {
         printf("Skipping symbol %s\n", symbol);
         free(alpha);
         continue;
      }

      letters[codepoint - ' '] = (letter_t) {
         .codepoint = codepoint,
         .width = width,
         .height = height,
         .alpha = alpha
      };
   } 
  
   free(line);
   return letters;
}

void draw_letter(image_t image, int x, int y, letter_t letter, rgb_t color, int scale) {
   for (int i = 0; i < letter.width; i++) {
      for (int j = 0; j < letter.height; j++) {
         unsigned char val = letter.alpha[j * letter.width + i];
         if (val > 0) {
            for (int ii = 0; ii < scale; ii++) {
               for (int jj = 0; jj < scale; jj++) {
                  set_pixel(image, x + i * scale + ii, y + j * scale + jj, color);
               }
            }
         }
      }
   }
}

void draw_text(image_t image, int x, int y, letter_t* letters, char* text, rgb_t color, int scale) {
   int offset = 0;
   for (int i = 0; text[i] != '\0'; i++) {
      draw_letter(image, x + offset, y, letters[text[i] - ' '], color, scale);
      offset += (letters[text[i] - ' '].width + 1) * scale;
   }
}

#endif /* #ifndef IMPL_RASTERIZE_H_DONE */
#endif /* #ifdef IMPL_RASTERIZE_H */
