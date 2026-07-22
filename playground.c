#include <stdio.h>
#include <sys/time.h>

#define IMPL_NOISE_H
#include "src/noise.h"

int main() {

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
