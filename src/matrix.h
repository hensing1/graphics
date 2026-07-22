#ifndef MATRIX_H
#define MATRIX_H

#include "util.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

typedef struct {
   size_t height;
   size_t width;
   float *data;
   size_t data_height;
   size_t data_width;
   size_t offset_i;
   size_t offset_j;
   bool transposed;
   int *ref_counter;
} matrix_t;

#define mat_val(m, i, j)                                                       \
   (m.transposed                                                               \
        ? m.data[m.data_height * ((j) + m.offset_j) + ((i) + m.offset_i)]      \
        : m.data[m.data_width * ((i) + m.offset_i) + ((j) + m.offset_j)])
#define mat_set(m, i, j, val)                                                  \
   (m.data[m.data_width * ((i) + m.offset_i) + ((j) + m.offset_j)] = (val))

matrix_t mat_init(size_t height, size_t width, float *data,
                  bool use_ref_counter);
matrix_t mat_zero(size_t height, size_t width);
matrix_t mat_full(size_t height, size_t width, float v);
matrix_t mat_eye(size_t height, size_t width);

matrix_t mat_submatrix(matrix_t m, size_t start_i, size_t end_i, size_t start_j,
                       size_t end_j);
matrix_t mat_clone(matrix_t m);

matrix_t mat_transpose(matrix_t m);
void mat_scale(matrix_t m, float d);
void mat_add(matrix_t a, matrix_t b);
matrix_t mat_mul(matrix_t a, matrix_t b);
matrix_t mat_outer(matrix_t a, matrix_t b);
float mat_dot(matrix_t a, matrix_t b);

float mat_l1(matrix_t m);
float mat_l2_squared(matrix_t m);
inline float mat_l2(matrix_t m);
float mat_trace(matrix_t m);

/*
 * Turns `m` into an upper triangular matrix r.
 * If `compute_q` is set, also returns matrix orthogonal matrix q such that
 * `m` = q * r.
 */
matrix_t mat_qr(matrix_t m, bool compute_q);

/*
 * Returns the solution x for `y` = `r` * x.
 * Requires that `r` is an upper triangular matrix.
 */
matrix_t back_substitute(matrix_t r, matrix_t y);

void mat_print(matrix_t m);

void mat_free(matrix_t m);

#endif /* #ifndef MATRIX_H */

#ifdef IMPL_MATRIX_H

#ifndef IMPL_MATRIX_H_DONE
#define IMPL_MATRIX_H_DONE

matrix_t mat_init(size_t height, size_t width, float *data,
                  bool use_ref_counter) {
   int *ref_counter;
   if (use_ref_counter) {
      ref_counter  = malloc(sizeof(int));
      *ref_counter = 1;
   } else {
      ref_counter = NULL;
   }
   return (matrix_t){.height      = height,
                     .width       = width,
                     .data        = data,
                     .data_height = height,
                     .data_width  = width,
                     .offset_i    = 0,
                     .offset_j    = 0,
                     .transposed  = false,
                     .ref_counter = ref_counter};
}

matrix_t mat_zero(size_t height, size_t width) {
   float *values = calloc(sizeof(float), height * width);
   if (!values) {
      char *err;
      asprintf(&err, "Could not allocate matrix of size %lux%lu", height,
               width);
      perror(err);
      free(err);
      exit(EXIT_FAILURE);
   }
   return mat_init(height, width, values, true);
}

matrix_t mat_full(size_t height, size_t width, float v) {
   matrix_t m = mat_zero(height, width);
   for (size_t i = 0; i < height; i++) {
      for (size_t j = 0; j < width; j++) { mat_set(m, i, j, v); }
   }
   return m;
}

matrix_t mat_eye(size_t height, size_t width) {
   matrix_t id = mat_zero(height, width);
   size_t k    = min(height, width);
   for (size_t i = 0; i < k; i++) { mat_set(id, i, i, 1); }
   return id;
}

matrix_t mat_transpose(matrix_t m) {
   matrix_t m_t = mat_submatrix(m, 0, m.height, 0, m.width);
   size_t tmp   = m_t.height;
   m_t.height   = m_t.width;
   m_t.width    = tmp;

   tmp             = m_t.data_height;
   m_t.data_height = m_t.data_width;
   m_t.data_width  = tmp;

   m_t.transposed = !m.transposed;

   return m_t;
}

matrix_t mat_submatrix(matrix_t m, size_t start_i, size_t end_i, size_t start_j,
                       size_t end_j) {
   assert(start_i < end_i);
   assert(start_j < end_j);
   assert(end_i <= m.offset_i + m.height);
   assert(end_j <= m.offset_j + m.width);

   if (m.ref_counter) { (*m.ref_counter)++; }
   return (matrix_t){.height      = end_i - start_i,
                     .width       = end_j - start_j,
                     .data        = m.data,
                     .data_height = m.data_height,
                     .data_width  = m.data_width,
                     .offset_i    = start_i,
                     .offset_j    = start_j,
                     .transposed  = m.transposed,
                     .ref_counter = m.ref_counter};
}

matrix_t mat_clone(matrix_t m) {
   float *cloned_data = malloc(sizeof(float) * m.height * m.width);
   int *ref_counter   = malloc(sizeof(int));
   *ref_counter       = 1;
   matrix_t clone     = {.height      = m.height,
                         .width       = m.width,
                         .data        = cloned_data,
                         .data_height = m.height,
                         .data_width  = m.width,
                         .offset_i    = 0,
                         .offset_j    = 0,
                         .transposed  = false,
                         .ref_counter = ref_counter};
   for (size_t i = 0; i < m.height; i++) {
      for (size_t j = 0; j < m.width; j++) {
         mat_set(clone, i, j, mat_val(m, i, j));
      }
   }
   return clone;
}

void mat_scale(matrix_t m, float d) {
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
         float val = 0;
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

float mat_dot(matrix_t a, matrix_t b) {
   assert(a.width == 1);
   assert(b.width == 1);
   assert(a.height == b.height);
   float res = 0;
   for (size_t i = 0; i < a.height; i++) {
      res += mat_val(a, i, 0) * mat_val(b, i, 0);
   }
   return res;
}

float mat_l1(matrix_t m) {
   assert(m.width == 1);
   float norm = 0;
   for (size_t i = 0; i < m.height; i++) { norm += fabsf(mat_val(m, i, 0)); }
   return norm;
}

float mat_l2_squared(matrix_t m) {
   assert(m.width == 1);
   float norm = 0;
   for (size_t i = 0; i < m.height; i++) {
      norm += mat_val(m, i, 0) * mat_val(m, i, 0);
   }
   return norm;
}

float mat_l2(matrix_t m) { return sqrtf(mat_l2_squared(m)); }

float mat_trace(matrix_t m) {
   assert(m.height == m.width);
   float sum = 0;
   for (size_t i = 0; i < m.height; i++) { sum += mat_val(m, i, i); }
   return sum;
}

matrix_t mat_qr(matrix_t m, bool compute_q) {
   matrix_t q_t;
   if (compute_q) { q_t = mat_eye(m.height, m.width); }
   for (size_t k = 0; k < m.width; k++) {
      matrix_t x   = mat_submatrix(m, k, m.height, k, k + 1);
      matrix_t v_k = mat_clone(x);
      mat_set(v_k, 0, 0, sgn(mat_val(x, 0, 0)) * mat_l2(x) + mat_val(x, 0, 0));
      mat_free(x);
      mat_scale(v_k, 1 / mat_l2(v_k));

      matrix_t lower_r = mat_submatrix(m, k, m.height, k, m.width);
      matrix_t v_k_t   = mat_transpose(v_k);
      matrix_t tmp     = mat_mul(v_k_t, lower_r);
      matrix_t tmp2    = mat_mul(v_k, tmp);
      mat_scale(tmp2, -2);
      mat_add(lower_r, tmp2);
      mat_free(lower_r);
      mat_free(tmp);
      mat_free(tmp2);

      if (compute_q) {
         matrix_t lower_q = mat_submatrix(q_t, k, q_t.height, 0, q_t.width);
         tmp              = mat_mul(v_k_t, lower_q);
         tmp2             = mat_mul(v_k, tmp);
         mat_scale(tmp2, -2);
         mat_add(lower_q, tmp2);
         mat_free(lower_q);
         mat_free(tmp);
         mat_free(tmp2);
      }

      mat_free(v_k);
      mat_free(v_k_t);
   }
   if (compute_q) {
      matrix_t q = mat_transpose(q_t);
      mat_free(q_t);
      return q;
   }
   return q_t;
}

matrix_t back_substitute(matrix_t r, matrix_t y) {
   assert(r.height == y.height);
   assert(r.height >= r.width);

   matrix_t solution = mat_zero(r.width, 1);
   for (int i = r.width - 1; i >= 0; i--) {
      float sum = 0;
      for (int j = r.width - 1; j > i; j--) {
         sum += mat_val(solution, j, 0) * mat_val(r, i, j);
      }
      mat_set(solution, i, 0, (mat_val(y, i, 0) - sum) / mat_val(r, i, i));
   }
   return solution;
}

void mat_print(matrix_t m) {
   printf("---\n");
   for (size_t i = 0; i < m.height; i++) {
      for (size_t j = 0; j < m.width; j++) { printf("%f  ", mat_val(m, i, j)); }
      printf("\n");
   }
   printf("---\n");
}

void mat_free(matrix_t m) {
   if (!m.ref_counter) { return; }
   (*m.ref_counter)--;
   if (*m.ref_counter <= 0) { // implicit check against double free
      free(m.ref_counter);
      free(m.data);
   }
}

#endif /* #ifndef IMPL_MATRIX_H_DONE */
#endif /* #ifdef IMPL_MATRIX_H */
