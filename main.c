#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMPL_COLOR_H
#define IMPL_IO_H
#define IMPL_MATRIX_H
#define IMPL_NOISE_H
#define IMPL_RASTERIZE_H

#include "src/color.h"
#include "src/io.h"
#include "src/matrix.h"
#include "src/noise.h"
#include "src/rasterize.h"

/// point lerp
point_t plerp(point_t p1, point_t p2, float u) {
   return (point_t){(1 - u) * p1.x + u * p2.x, (1 - u) * p1.y + u * p2.y};
}

void draw_vector(image_t image, point_t start, point_t vec) {
   point_t end = {round(start.x + vec.x), round(start.y + vec.y)};
   draw_line(image, start, end, WHITE);
   draw_cross(image, end.x, end.y, RED);
}

void draw_bezier(image_t image, point_t p1, point_t p2, point_t p3,
                 point_t p4) {
   draw_cross(image, p1.x, p1.y, RED);
   draw_cross(image, p2.x, p2.y, RED);
   draw_cross(image, p3.x, p3.y, RED);
   draw_cross(image, p4.x, p4.y, RED);

   for (float u = 0; u <= 1; u += 1.f / 256) {
      // de casteljau's algorithm
      point_t a1 = plerp(p1, p2, u);
      point_t a2 = plerp(p2, p3, u);
      point_t a3 = plerp(p3, p4, u);

      point_t b1 = plerp(a1, a2, u);
      point_t b2 = plerp(a2, a3, u);

      point_t c1 = plerp(b1, b2, u);
      set_pixel(image, round(c1.x), round(c1.y), WHITE);
   }
}

void draw_hermite(image_t image, point_t p1, point_t t1, point_t p2,
                  point_t t2) {
   draw_vector(image, p1, t1);
   draw_vector(image, p2, t2);

   point_t prev = p1;
   for (double u = 1.f / 128; u <= 1; u += 1.f / 128) {
      double h0 = (1 - u) * (1 - u) * (1 + 2 * u);
      double h1 = 10 * u * (1 - u) * (1 - u);
      double h2 = 10 * -(u * u) * (1 - u);
      double h3 = (3 - 2 * u) * u * u;

      point_t next = {round(h0 * p1.x + h1 * t1.x + h2 * t2.x + h3 * p2.x),
                      round(h0 * p1.y + h1 * t1.y + h2 * t2.y + h3 * p2.y)};
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
      if (norm(z) > 4) { return i; }
   }
   return 0;
}

void mandelbrot(image_t image) {
   int iterations      = 64;
   double aspect_ratio = ((double)image.height) / image.width;
   for (int y = 0; y < image.height; y++) {
      for (int x = 0; x < image.width; x++) {
         long double complex c =
             (3. * x / image.width - 2) +
             (aspect_ratio * (3. * y / image.height - 1.5)) * I;
         uint8_t d;
         if ((d = diverges(c, iterations))) {
            float u     = d / (float)iterations;
            u           = pow(u, 0.4f); // gamma correction
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
      for (int j = 0; j < image.height; j++) { set_pixel(image, i, j, c); }
   }
}

void bezier_gradient(image_t image, rgb_t col1, rgb_t col2, rgb_t col3) {
   for (int i = 0; i < image.width; i++) {
      float t = (float)i / (image.width - 1);
      rgb_t c = rgb_bezier_quadratic(col1, col2, col3, t);
      for (int j = 0; j < image.height; j++) { set_pixel(image, i, j, c); }
   }
}

bool wolfberd(bool *neighbors, uint8_t rule) {
   int n        = neighbors[-1] * 4 + neighbors[0] * 2 + neighbors[1];
   uint8_t mask = 1 << n;
   return (mask & rule) > 0;
}

void draw_a_wolfberd(image_t image, uint8_t rule, bool random_init) {
   bool *state      = malloc(sizeof(bool) * (image.width + 2));
   bool *next_state = malloc(sizeof(bool) * (image.width + 2));
   if (random_init) {
      timeseed_prng();
      for (int i = 1; i < image.width + 1; i++) { state[i] = prng_next() & 1; }
   } else {
      state[image.width / 2 + 1] = true;
   }
   for (int y = 0; y < image.height; y++) {
      for (int x = 0; x < image.width; x++) {
         if (state[x + 1]) { set_pixel(image, x, y, WHITE); }
      }
      state[0]               = state[image.width];
      state[image.width + 1] = state[1];
      for (int x = 1; x < image.width + 1; x++) {
         next_state[x] = wolfberd((state + x), rule);
      }
      bool *tmp  = state;
      state      = next_state;
      next_state = tmp;
   }
   free(state);
   free(next_state);
}

typedef enum { EMPTY, UP, RIGHT, DOWN, LEFT } CellState;

#define CELL(maze, x, y, width) (maze[(y) * (width) + (x)])
#define MAZE_PATH               WHITE
#define MAZE_WALL               BLACK
static int maze_exit = -1;

void permute(CellState *dirs) {
   for (int i = 0; i < 3; i++) {
      int other = randInt(i, 4);
      if (i == other) continue;
      CellState tmp = dirs[i];
      dirs[i]       = dirs[other];
      dirs[other]   = tmp;
   }
}

void maze_dfs(int x, int y, CellState *maze, size_t width, size_t height,
              CellState dir) {
   CELL(maze, x, y, width) = dir;
   CellState dirs[4]       = {UP, RIGHT, DOWN, LEFT};
   permute(dirs);
   for (int i = 0; i < 4; i++) {
      CellState dir = dirs[i];
      // if (CELL(maze, x, y, width) == dir) continue;

      int dx, dy;
      switch (dir) {
         case UP:
            dx = 0;
            dy = 1;
            break;
         case RIGHT:
            dx = -1;
            dy = 0;
            break;
         case DOWN:
            dx = 0;
            dy = -1;
            break;
         case LEFT:
            dx = 1;
            dy = 0;
            break;
         default:
            assert(false);
      }

      int new_x = x + dx, new_y = y + dy;
      if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height ||
          CELL(maze, new_x, new_y, width) != EMPTY) {
         continue;
      }
      maze_dfs(new_x, new_y, maze, width, height, dir);
   }
   if (y == height - 1 && maze_exit == -1) { maze_exit = x; }
}

void make_maze(size_t height, size_t width) {
   CellState *maze = calloc(height * width, sizeof(CellState));
   timeseed_prng();
   uint64_t start = prng_next() % width;
   maze_dfs(start, 0, maze, width, height, UP);

   rgb_t *pixels =
       (rgb_t *)malloc(sizeof(rgb_t) * (2 * width + 1) * (2 * height + 1));
   image_t image = {2 * width + 1, 2 * height + 1, pixels};
   set_uniform(image, MAZE_WALL);

   for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
         int px = 2 * x + 1, py = 2 * y + 1;
         set_pixel(image, px, py, MAZE_PATH);
         switch (CELL(maze, x, y, width)) {
            case UP:
               set_pixel(image, px, py - 1, MAZE_PATH);
               break;
            case RIGHT:
               set_pixel(image, px + 1, py, MAZE_PATH);
               break;
            case DOWN:
               set_pixel(image, px, py + 1, MAZE_PATH);
               break;
            case LEFT:
               set_pixel(image, px - 1, py, MAZE_PATH);
               break;
            default:
               assert(false);
         }
      }
   }
   set_pixel(image, 2 * maze_exit + 1, 2 * height, MAZE_PATH);

   save_to_ppm(image, "pics/smolmaze.ppm");
   free(image.pixels);
   free(maze);
}

void checkerboard(const image_t img, const size_t square_size) {
   for (size_t x = 0; x < img.width / square_size; x++) {
      for (size_t y = 0; y < img.height / square_size; y++) {
         for (size_t i = 0; i < square_size; i++) {
            for (size_t j = 0; j < square_size; j++) {
               set_pixel(img, square_size * x + i, square_size * y + j, (x + y) % 2 == 0 ? WHITE : BLACK);
            }
         }
      }
   }
}

void stripes(const image_t img, const size_t stripe_width) {
   for (size_t x = 0; x < img.width / stripe_width; x++) {
      for (size_t y = 0; y < img.height; y++) {
         for (size_t i = 0; i < stripe_width; i++) {
            set_pixel(img, stripe_width * x + i, y, x % 2 == 0 ? WHITE : BLACK);
         }
      }
   }
}

int main(int argc, char *argv[]) {
   // const int width = 65 * 8;
   // const int height = 8 * 65;
   // const int height = 33;

   char filename[] = "pics/beemoviescript.ppm";

   letter_t* letters = load_bs("./assets/fonts/verytiny.bs");

   // char text['~' - ' ' + 2];
   int maxWidth = 0;
   // char* text = "I drink, therefore I don't think, therefore I don't am.";
   // char* text = "sphinx of black quartz, judge my vow";
   // char* text = "franz jagt im komplett verwahrlosten taxi quer durch bayern";
   // char* lines[] = {
   //    "According to all known laws of aviation, there is no way a bee should be able to fly.",
   //    "Its wings are too small to get its fat little body off the ground.",
   //    "The bee, of course, flies anyway because bees don't care what humans think is impossible.",
   //    "Yellow, black. Yellow, black. Yellow, black. Yellow, black."
   // };
   FILE* fp = fopen("./assets/beemovie.txt", "r");
   char* lines[1363];
   size_t len = 0;
   for (int i = 0; i < 1363; i++) {
      ssize_t r = getline(&lines[i], &len, fp);
      lines[i][r-1] = 0;
   }
   for (int i = 0; i < 1363; i++){
      int currWidth = 0;
      for (int j = 0; j < strlen(lines[i]); j++)
         currWidth += letters[lines[i][j] - '!'+1].width + 1;
      maxWidth = max(maxWidth, currWidth);
   }
   // for (char c = '!'; c <= '~'; c++) {
   //    text[c - '!'] = c;
   //    width += letters[c - '!' + 1].width + 1;
   // }
   // text['~'-' '+1] = '\0';

   maxWidth--;
   // maxWidth *= 3;
   maxWidth += 6;

   int height = 1363 * 5 + 1362 * 3 + 2 * 3;

   rgb_t* pixels = (rgb_t*)malloc(sizeof(rgb_t) * maxWidth * height);
   image_t image = {maxWidth, height, pixels};
   set_uniform(image, BLACK);
   for (int i = 0; i < 1363; i++) {
      draw_text(image, 3, 3 + 8*i, letters, lines[i], YELLOW, 1);
   }

   save_to_ppm(image, filename);

   for (int i = 0; i < '~' - ' ' + 1; i++) {
      if (letters[i].alpha) { free(letters[i].alpha); }
   }
   free(letters);
   free(image.pixels);

   printf("Written %s.\n", filename);

   return 0;
}
