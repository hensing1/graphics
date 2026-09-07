#ifndef COLOR_H
#define COLOR_H

#include "maths.h"

typedef struct {
   float r;
   float g;
   float b;
} rgb_t;

typedef struct {
   float h;
   float s;
   float v;
} hsv_t;

#define BLACK (rgb_t){0., 0., 0.}
#define BLUE  (rgb_t){0., 0., 1.}
#define GREEN (rgb_t){0., 1., 0.}
#define RED   (rgb_t){1., 0., 0.}
#define WHITE (rgb_t){1., 1., 1.}

rgb_t rgb_lerp(rgb_t a, rgb_t b, float u);
rgb_t rgb_bezier_cubic(rgb_t c1, rgb_t c2, rgb_t c3, rgb_t c4, float u);
rgb_t rgb_bezier_quadratic(rgb_t c1, rgb_t c2, rgb_t c3, float u);

hsv_t rgb2hsv(rgb_t rgb);
rgb_t hsv2rgb(hsv_t hsv);

#endif /* COLOR_H */

/* ---- IMPLEMENTATION ---- */

#ifdef IMPL_COLOR_H
#ifndef IMPL_COLOR_H_DONE
#define IMPL_COLOR_H_DONE

rgb_t rgb_lerp(rgb_t a, rgb_t b, float u) {
   return (rgb_t){
       (1 - u) * a.r + u * b.r,
       (1 - u) * a.g + u * b.g,
       (1 - u) * a.b + u * b.b,
   };
}

rgb_t rgb_bezier_cubic(rgb_t c1, rgb_t c2, rgb_t c3, rgb_t c4, float u) {
   rgb_t a1 = rgb_lerp(c1, c2, u);
   rgb_t a2 = rgb_lerp(c2, c3, u);
   rgb_t a3 = rgb_lerp(c3, c4, u);

   rgb_t b1 = rgb_lerp(a1, a2, u);
   rgb_t b2 = rgb_lerp(a2, a3, u);

   return rgb_lerp(b1, b2, u);
}

rgb_t rgb_bezier_quadratic(rgb_t c1, rgb_t c2, rgb_t c3, float u) {
   rgb_t a1 = rgb_lerp(c1, c2, u);
   rgb_t a2 = rgb_lerp(c2, c3, u);

   return rgb_lerp(a1, a2, u);
}

hsv_t rgb2hsv(rgb_t rgb) {
   hsv_t hsv;
   float min_rgb = min(min(rgb.r, rgb.g), rgb.b);
   float max_rgb = max(max(rgb.r, rgb.g), rgb.b);

   float span = max_rgb - min_rgb;
   if (span == 0) { // r == g == b
      hsv.h = 0;
   } else if (max_rgb == rgb.r) {
      hsv.h = (rgb.g - rgb.b) / (6 * span);
   } else if (max_rgb == rgb.g) {
      hsv.h = (2 + ((rgb.g - rgb.b) / span)) / 6;
   } else if (max_rgb == rgb.b) {
      hsv.h = (4 + ((rgb.r - rgb.g) / span)) / 6;
   }

   if (hsv.h < 0) { hsv.h += 1.; }

   hsv.s = span == 0 ? 0 : span / max_rgb;
   hsv.v = max_rgb;

   return hsv;
}

rgb_t hsv2rgb(hsv_t hsv) {
   rgb_t rgb;
   if (hsv.s == 0) {
      rgb.r = rgb.g = rgb.b = hsv.v;
      return rgb;
   }
   int h   = hsv.h * 6;
   float f = ((hsv.h * 6) - ((int)(hsv.h * 6)));
   float p = hsv.v * (1 - hsv.s);
   float q = hsv.v * (1 - hsv.s * f);
   float t = hsv.v * (1 - hsv.s * (1 - f));

   switch (h) {
      case 0:
      case 6:
         rgb = (rgb_t){hsv.v, t, p};
         break;
      case 1:
         rgb = (rgb_t){q, hsv.v, p};
         break;
      case 2:
         rgb = (rgb_t){p, hsv.v, t};
         break;
      case 3:
         rgb = (rgb_t){p, q, hsv.v};
         break;
      case 4:
         rgb = (rgb_t){t, p, hsv.v};
         break;
      case 5:
         rgb = (rgb_t){hsv.v, p, q};
         break;
   }
   return rgb;
}

#endif /* #ifndef IMPL_COLOR_H_DONE */
#endif /* #ifdef IMPL_COLOR_C */
