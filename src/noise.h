#ifndef NOISE_H
#define NOISE_H

#include <stdint.h>
#include <sys/time.h>

/*
 * A stratified sampling technique for the interval [0, 1).
 *
 * Radical inverse of a number i: digits of i in base b, reversed, after the decimal point.
 * e.g. radicalInverse(2, 6): 6 -> 110 -> 011 -> 0.011 = 0.375
 */
float radicalInverse(int base, int num);

void seed_prng(uint64_t seed);
void timeseed_prng();
uint64_t prng_next();

#endif /* #ifndef NOISE_H */

#ifdef IMPL_NOISE_H

static uint64_t _blum_micali_state = 42;
static uint64_t _blum_micali_modulus = 310091; // some "large" prime for which 2 is a primitive root
// static uint64_t _blum_micali_modulus = 22399099;
static uint64_t _xorshift_state = 1;


static uint64_t modular_exp(uint64_t base, uint64_t exp, uint64_t modulus) {
    uint64_t res = base;
    for (uint64_t i = 0; i < exp - 1; i++) {
        res *= base;
        res %= modulus;
    }
    return res;
}

static uint64_t modular_exp2(uint64_t exp, uint64_t modulus) {
    uint64_t res = 2;
    for (uint64_t i = 0; i < exp - 1; i++) {
        res <<= 1;
        res %= modulus;
    }
    return res;
}

void seed_blum_micali(uint64_t seed) {
    _blum_micali_state = seed % _blum_micali_modulus;
}

// xorshift64s, variant A_1(12,25,27) with multiplier M_32 from line 3 of table 5
// https://en.wikipedia.org/wiki/Xorshift#xorshift*
uint64_t xorshift64star() {
    _xorshift_state ^= _xorshift_state >> 12;
    _xorshift_state ^= _xorshift_state << 25;
    _xorshift_state ^= _xorshift_state >> 27;
    return _xorshift_state * 0x2545F4914F6CDD1DULL;
}

void seed_prng(uint64_t seed) {
    _xorshift_state = seed;
}

uint64_t prng_next() {
    // uint64_t mask = (1 << num_bits) - 1;
    // _blum_micali_state = modular_exp2(
    //     _blum_micali_state,
    //     _blum_micali_modulus
    // );
    // return _blum_micali_state & mask;
    return xorshift64star();
}

void timeseed_prng() {
    struct timeval now;
    gettimeofday(&now, (void*) 0);
    seed_prng(now.tv_usec);
}

float radicalInverse(int base, int num) {
    int inverse = 0;
    int p = 1;

    while (num > 0) {
        inverse *= base;
        inverse += num % base;
        p *= base;
        num /= base;
    }

    return inverse / (float) p;
}

#endif /* #ifdef IMPL_NOISE_H */
