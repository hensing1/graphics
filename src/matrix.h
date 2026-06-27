#ifndef MATRIX_H
#define MATRIX_H

#include "util.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  size_t height;
  size_t width;
  double* values;
} matrix_t;


#define mat_val(m, i, j) (m.values[m.width * (i) + (j)])
#define mat_set(m, i, j, val) (m.values[m.width * (i) + (j)] = (val))

matrix_t mat_zero(size_t height, size_t width);
matrix_t mat_full(size_t height, size_t width, double v);
matrix_t mat_eye(size_t height, size_t width);
void mat_scale(double d, matrix_t m);
void mat_add(matrix_t a, matrix_t b);
matrix_t mat_mul(matrix_t a, matrix_t b);
matrix_t mat_outer(matrix_t a, matrix_t b);
double mat_inner(matrix_t a, matrix_t b);
void mat_print(matrix_t m);
void mat_free(matrix_t m);

#endif  /* #ifndef MATRIX_H */

#ifdef IMPL_MATRIX_H

matrix_t mat_zero(size_t height, size_t width) {
  double* values;
  values = (double*)calloc(sizeof(double), height * width);
  if (!values) {
    char* err;
    asprintf(&err, "Could not allocate matrix of size %lux%lu", height, width);
    perror(err);
    free(err);
    exit(EXIT_FAILURE);
  }
  return (matrix_t){height=height, width=width, values=values};
}

matrix_t mat_full(size_t height, size_t width, double v) {
  matrix_t m = mat_zero(height, width);
  for (size_t i = 0; i < height; i++) {
    for (size_t j = 0; j < width; j++) {
      mat_set(m, i, j, v);
    }
  }
  return m;
}

matrix_t mat_eye(size_t height, size_t width) {
  matrix_t I = mat_zero(height, width);
  size_t k = min(height, width);
  for (size_t i = 0; i < k; i++) {
    mat_set(I, i, i, 1);
  }
  return I;
}

void mat_scale(double d, matrix_t m) {
  for (size_t i = 0; i < m.height; i++) {
    for (size_t j = 0; j < m.width; j++) {
       mat_set(m, i, j, d * mat_val(m, i, j));
    }
  }
}

void mat_add(matrix_t a, matrix_t b) {
  assert(a.height == b.height);
  assert(a.width == b.width);
  for (size_t i = 0; i < a.height; i++) {
    for (size_t j = 0; j < a.width; j++) {
       mat_set(a, i, j, mat_val(a, i, j) + mat_val(b, i, j));
    }
  }
}

matrix_t mat_mul(matrix_t a, matrix_t b) {
  assert(a.width == b.height);
  matrix_t new = mat_zero(a.height, b.width);
  for (size_t i = 0; i < new.height; i++) {
    for (size_t j = 0; j < new.width; j++) {
      double val = 0;
      for (size_t k = 0; k < a.width; k++) {
        val += mat_val(a, i, k) * mat_val(b, k, j);
      }
      mat_set(new, i, j, val);
    }
  }
  return new;
}

matrix_t mat_outer(matrix_t a, matrix_t b) {
  assert(a.width == 1);
  assert(b.width == 1);
  matrix_t new = mat_zero(a.height, b.height);
  for (size_t i = 0; i < new.height; i++) {
    for (size_t j = 0; j < new.width; j++) {
      mat_set(new, i, j, mat_val(a, i, 0) * mat_val(b, j, 0));
    }
  }
  return new;
}

double mat_inner(matrix_t a, matrix_t b) {
  assert(a.width == 1);
  assert(b.width == 1);
  assert(a.height == b.height);
  double res = 0;
  for (size_t i = 0; i < a.height; i++) {
    res += mat_val(a, i, 0) * mat_val(b, i, 0);
  }
  return res;
}

void mat_print(matrix_t m) {
  for (size_t i = 0; i < m.height; i++) {
    for (size_t j = 0; j < m.width; j++) {
      printf("%f  ", mat_val(m, i, j));
    }
    printf("\n");
  }
}

void mat_free(matrix_t m) {
  free(m.values);
}

#endif /* #ifdef IMPL_MATRIX_H */
