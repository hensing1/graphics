#ifndef NOISE_H
#define NOISE_H

/*
 * A stratified sampling technique for the interval [0, 1).
 *
 * Radical inverse of a number i: digits of i in base b, reversed, after the decimal point.
 * e.g. radicalInverse(2, 6): 6 -> 110 -> 011 -> 0.011 = 0.375
 */
float radicalInverse(int base, int num);

#endif /* #ifndef NOISE_H */

#ifdef IMPL_NOISE_H

float radicalInverse(int base, int num) {
    int inverse = 0;
    int p = 1;

    while (num > 0) {
        inverse *= base;
        inverse += num % base;
        p *= base;
        num /= base;
    }

    return (float) inverse / (float) p;
}

#endif /* #ifdef IMPL_NOISE_H */
