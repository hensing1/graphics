#include <stdio.h>
#include <stdlib.h>

static inline float numeral(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }
  return -1;
}

float parsenumbersegment(char** c) {
  float num = 0;
  float d;
  while ((d = numeral(**c)) != -1) {
    num *= 10;
    num += d;
    (*c)++;
  }
  return num;
}

float exp10(int num) {
  if (num == 0) {
    return 1;
  }
  if (num < 0) {
    return 1 / exp10(-num);
  }
  float h = exp10(num / 2);
  if (num % 2 == 1) {
    return h * h * 10;
  }
  else {
    return h * h;
  }
}

float tonumber(char* nString) {
  float sgn = nString[0] == '-' ? -1 : 1;
  if (nString[0] == '-') {nString++;}
  
  float base = parsenumbersegment(&nString);
  float decimals = 0;
  if (*nString == '.') {
    nString++;
    char* start = nString;
    decimals = parsenumbersegment(&nString);
    int decimalLength = nString - start;
    decimals /= exp10(decimalLength);
  }
  float mult = 1;
  if (*nString == 'e' || *nString == 'E') {
    nString++;
    float expSgn = *nString == '-' ? -1 : 1;
    if (nString[0] == '+' || nString[0] == '-') {nString++;}
    mult = exp10(expSgn * parsenumbersegment(&nString));
  }
  printf("Parsed numbers: (%f + %f) * %f\n", sgn * base, decimals, mult);
  return sgn * (base + decimals) * mult;
}

int main() {
  size_t n = 1024;
  char* buf = malloc(n);
  while (buf[0] != EOF) {
    getline(&buf, &n, stdin);
    printf("%f\n", tonumber(buf));
  }
  return 0;
}
