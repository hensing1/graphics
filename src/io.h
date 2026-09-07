#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdio.h>

#include "maths.h"
#include "rasterize.h"

void save_to_ppm(image_t image, char *filename);

#endif /* #ifndef IO_H */

#ifdef IMPL_IO_H
#ifndef IMPL_IO_H_DONE
#define IMPL_IO_H_DONE

void save_to_ppm(image_t image, char *filename) {
   FILE *f = fopen(filename, "wb");

   fprintf(f, "P6\n"); // magic number for ppm format
   fprintf(f, "%d %d\n", image.width, image.height);
   fprintf(f, "255\n"); // max color value

   for (int y = 0; y < image.height; y++) {
      for (int x = 0; x < image.width; x++) {
         rgb_t p = get_pixel(image, x, y);
         fputc((uint8_t)round(p.r * 255), f);
         fputc((uint8_t)round(p.g * 255), f);
         fputc((uint8_t)round(p.b * 255), f);
      }
   }

   fclose(f);
}

#endif /* #ifdef IMPL_IO_H_DONE */
#endif /* #ifdef IMPL_IO_H */
