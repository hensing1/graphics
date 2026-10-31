#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define IMPL_NOISE_H
#include "src/noise.h"

#include <math.h>

#define IMPL_MATHS_H
#include "src/maths.h"

#include "src/datastructures.h"
#undef DEFAULT_ARRAY_CAPACITY
#define DEFAULT_ARRAY_CAPACITY 4
DEFINE_ARRAY(int, int)
IMPLEMENT_ARRAY(int, int)

#undef DEFAULT_ARRAY_CAPACITY
#define DEFAULT_ARRAY_CAPACITY 12
DEFINE_ARRAY(char*, string)
IMPLEMENT_ARRAY(char*, string)

void the_e() {
   // printf("%f\n", exp(1));
   // printf("%f\n", expf(1));
   printf("%f\n", ln(E));
   printf("%f\n", logf(E));
   printf("%f %f\n", logf(1), ln(exp(0)));
   printf("%f %f\n", logf(1/E), ln(exp(-1)));
   printf("%f %f\n", logf(1/(E*E)), ln(exp(-2)));
   printf("%f %f\n", logf(1/(E*E*E)), ln(exp(-3)));
   printf("%f %f\n", logf(1/(E*E*E*E)), ln(exp(-4)));
   printf("%f %f\n", logf(1/(E*E*E*E*E)), ln(exp(-5)));
   printf("%f %f\n", logf(1/(E*E*E*E*E*E)), ln(exp(-6)));
   printf("%f %f\n", logf(1/(E*E*E*E*E*E*E)), ln(exp(-7)));
   printf("%f %f\n", logf(1/(E*E*E*E*E*E*E*E)), ln(exp(-8)));
   printf("%f %f\n", logf(1/(E*E*E*E*E*E*E*E*E)), ln(exp(-9)));
}

void the_pow() {
   printf("%i\n", pow(2, 3));
   printf("%f\n", pow(2.5f, 4));
   printf("%f\n", pow(2.f, 2.f));
   printf("%f\n", pow(2.f, 0.5f));
   printf("%f\n", sqrt(2));
}

int main() {
   int_array_t arr = int_array_init();
   int_array_insert(&arr, 1, 0);
   int_array_insert(&arr, 1, 0);
   int_array_insert(&arr, 1, 0);
   int_array_insert(&arr, 1, 0);
   int_array_insert(&arr, 2, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 4, 0);
   int_array_insert(&arr, 9, 0);
   int_array_insert(&arr, 2, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 4, 0);
   int_array_insert(&arr, 9, 0);
   int_array_insert(&arr, 2, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 4, 0);
   int_array_insert(&arr, 9, 0);
   int_array_insert(&arr, 2, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 3, 0);
   int_array_insert(&arr, 4, 0);
   int_array_insert(&arr, 9, 0);
   printf("%i\n", int_array_pop(&arr));
   printf("%i\n", int_array_pop(&arr));
   printf("%i\n", int_array_pop(&arr));
   printf("%i\n", int_array_pop(&arr));
   printf("%i\n", int_array_pop(&arr));
   for (int i = 0; i < arr.count; i++) {
      printf("%i\n", arr.items[i]);
   }
   printf("capacity of int array: %i\n", arr.capacity);

   string_array_t string_arr = string_array_init();
   string_array_push(&string_arr, "güten mürgen");
   printf("capacity of string array: %i\n", string_arr.capacity);

   return 0;

   the_pow();
   return 0;

   struct timeval now, stop;
   gettimeofday(&stop, NULL);
   int test_len = 1;
   stop.tv_sec += test_len;
   long count = 0;

   seed_prng(stop.tv_usec);
   do {
      prng_next();
      ++count;
      gettimeofday(&now, NULL);
   } while (now.tv_sec < stop.tv_sec ||
            (now.tv_sec == stop.tv_sec && now.tv_usec < stop.tv_usec));

   float elapsed = (now.tv_sec - (stop.tv_sec - test_len)) +
                   (now.tv_usec - stop.tv_usec) / 1000000;
   printf("Did %.2f prng's per second (tested for %.2f seconds).\n",
          count / elapsed, elapsed);

   for (int i = 0; i < 25; i++) {
      printf("%lu", prng_next() & 1);
      fflush(stdout);
   }
   printf("\n");

   return 0;
}
