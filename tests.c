#include <assert.h>
#include <stdbool.h>

#define IMPL_MATRIX_H
#include "src/matrix.h"

#define IMPL_NOISE_H
#include "src/noise.h"

#define do_test(test_fn)                                                       \
   test_fn();                                                                  \
   printf("Test passed: %s\n", #test_fn)

void test_mat_zero() {
   matrix_t m = mat_zero(4, 3);
   assert(m.height == 4);
   assert(m.width == 3);
   for (size_t i = 0; i < m.height; i++) {
      for (size_t j = 0; j < m.width; j++) { assert(mat_val(m, i, j) == 0); }
   }
   mat_free(m);
}

void test_mat_transpose() {
   matrix_t m = mat_zero(2, 3);
   mat_set(m, 0, 0, 1);
   mat_set(m, 0, 1, 2);
   mat_set(m, 0, 2, 3);
   mat_set(m, 1, 0, 4);
   mat_set(m, 1, 1, 5);
   mat_set(m, 1, 2, 6);
   // mat_print(m);
   matrix_t m_t = mat_transpose(m);
   // mat_print(m_t);
   assert(m_t.transposed);
   assert(m_t.height == 3);
   assert(m_t.width == 2);
   assert(mat_val(m_t, 0, 0) == 1);
   assert(mat_val(m_t, 0, 1) == 4);
   assert(mat_val(m_t, 1, 0) == 2);
   assert(mat_val(m_t, 1, 1) == 5);
   assert(mat_val(m_t, 2, 0) == 3);
   assert(mat_val(m_t, 2, 1) == 6);

   matrix_t m_t_t = mat_transpose(m_t);
   assert(!m_t_t.transposed);
   mat_free(m);
   mat_free(m_t);
   mat_free(m_t_t);
}

void test_mat_submatrix() {
   matrix_t m = mat_zero(1, 12);
   for (size_t i = 0; i < 12; i++) { mat_set(m, 0, i, i + 1); }
   m.height = m.data_height = 3;
   m.width = m.data_width = 4;
   mat_print(m);

   matrix_t sub = mat_submatrix(m, 1, 3, 1, 4);

   mat_print(sub);
   mat_free(sub);
   mat_free(m);
}

void test_mat_norm() {
   matrix_t m = mat_zero(3, 1);
   mat_set(m, 0, 0, 2);
   mat_set(m, 1, 0, -3);
   mat_set(m, 2, 0, 4);
   float len = mat_l2(m);
   assert(absf(len - 5.385164807134504) < 1e-6);
   assert(mat_l1(m) == 9);
   mat_free(m);
}

void test_mat_qr() {
   matrix_t m = mat_zero(9, 1);
   for (int i = 0; i < 9; i++) { mat_set(m, i, 0, i + 1); }
   m.height = m.data_height = 3;
   m.width = m.data_width = 3;
   matrix_t r             = mat_clone(m);
   matrix_t q             = mat_qr(r, true);

   matrix_t m_rec = mat_mul(q, r);

   for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
         assert(absf(mat_val(m, i, j) - mat_val(m_rec, i, j)) < 1e-5);
      }
   }
   matrix_t col1 = mat_submatrix(q, 0, 3, 0, 1);
   matrix_t col2 = mat_submatrix(q, 0, 3, 1, 2);
   matrix_t col3 = mat_submatrix(q, 0, 3, 2, 3);
   assert(absf(mat_dot(col1, col2)) < 1e-6);
   assert(absf(mat_dot(col2, col3)) < 1e-6);
   assert(absf(mat_dot(col3, col1)) < 1e-6);

   mat_free(q);
   mat_free(r);
   mat_free(m);
   mat_free(m_rec);
   mat_free(col1);
   mat_free(col2);
   mat_free(col3);
}

void test2_mat_qr() {
   matrix_t m = mat_zero(1, 8);
   for (int i = 0; i < 8; i++) { mat_set(m, 0, i, i); }
   m.height = m.data_height = 4;
   m.width = m.data_width = 2;
   mat_print(m);

   matrix_t r = mat_clone(m);
   matrix_t q = mat_qr(r, true);
   mat_print(q);
   mat_print(r);
   matrix_t res = mat_mul(q, r);
   mat_print(res);

   mat_free(m);
   mat_free(q);
   mat_free(r);
   mat_free(res);
}

void test_radical_inverse() {
   for (int i = 0; i <= 16; i++) {
      printf("Phi(%i) = %f\n", i, radicalInverse(2, i));
   }
}

void test_modular_exp() { assert(modular_exp(2, 5342, 22399099) == 3507239); }

int main() {
   do_test(test_mat_zero);
   do_test(test_mat_transpose);
   // do_test(test_radical_inverse);
   // do_test(test_mat_submatrix);
   do_test(test_mat_norm);
   do_test(test_mat_qr);
   // do_test(test2_mat_qr);
   do_test(test_modular_exp);
}
