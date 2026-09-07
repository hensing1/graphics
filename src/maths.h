#ifndef MATHS_H
#define MATHS_H

#define E 2.718281828459045
#define PI 3.141592653589793
#define TAU 6.283185307179586

#define absf h1_absf
#define ceil h1_ceilf
#define clamp h1_clampf
#define exp h1_expf
#define floor h1_floorf
#define inf h1_inff()
#define ln h1_lnf
#define max(a, b) \
    _Generic((a), \
        int: _Generic((b), int: h1_maxi, float: h1_maxf), \
        float: h1_maxf)(a, b)
#define min(a, b) \
    _Generic((a), \
        int: _Generic((b), int: h1_mini, float: h1_minf), \
        float: h1_minf)(a, b)
#define nan h1_nanf()
#define pow(a, b) \
    _Generic((b), \
        int: _Generic((a), int: h1_powii, float: h1_powfi),\
        float: h1_powf)(a, b)
#define round h1_roundf
#define sgn h1_sgnf
#define sqrt h1_sqrtf

float h1_absf(float f);
float h1_ceilf(float f);
float h1_clampf(float f, float low, float high);
float h1_expf(float x);
float h1_expm1f(float x);
float h1_floorf(float f);
float h1_inff();
float h1_lnf(float x);
float h1_maxf(float a, float b);
int   h1_maxi(int a, int b);
float h1_minf(float a, float b);
int   h1_mini(int a, int b);
float h1_nanf();
float h1_powf(float a, float b);
float h1_powfi(float a, int b);
int   h1_powii(int a, int b);
int   h1_roundf(float f);
int   h1_sgnf(float f);
float h1_sqrtf(float f);

#endif  // MATHS_H

#ifdef IMPL_MATHS_H

#ifndef IMPL_MATHS_H_DONE
#define IMPL_MATHS_H_DONE

float h1_absf(float f) {
    float negZero = -0.f;
    int abs = *(int*)&f&~*(int*)&negZero;
    return *(float*)&abs;
}

float h1_ceilf(float f) {
    float res = f - (int)f;
    if (res == 0) { return f; }
    if (f < 0) { return (int)f; }
    return (int) f + 1;
}

float h1_clampf(float f, float low, float high) {
  return h1_maxf(low, h1_minf(f, high));
}

float h1_expf(float x) {
    return 1 + h1_expm1f(x);
}

float h1_expm1f(float x) {
    float res = 0;
    float num = x;
    float denom = 1;
    int i = 1;
    float frac = num/denom;
    while (absf(frac) > 1e-6) {  // pretty sure the 1e-6 doesn't actually do anything
        res += frac;             // since `denom` will blow up to infty anyways
        num *= x;
        denom *= ++i;
        frac = num/denom;
    }
    return res;
}

float h1_floorf(float f) {
    if (f < 0) { return (int)f - 1; }
    return (int)f;
}

float h1_inff() {
    int n = 0x7f800000;
    return *(float*)&n;
}

/** Algorithm due to https://quadst.rip/ln-approx. It's bad and cursed, but at least fast
 *  This means that it doesn't actually fit well into the spirit of this repo, which is
 *  that everything be bad and cursed, but also slow */
float h1_lnf(float x) {
    int bits = *(int*)&x;
    int exponent = (bits >> 23) - 127;
    int mantissa = (bits & 0x007fffff) | 0x3f800000;
    x = *(float*)&mantissa;
    // return -1.49278+(2.11263+(-0.729104+0.10969*x)*x)*x+0.6931471806*exponent;
    // float a = -1.7417939+(2.8212026+(-1.4699568+(0.44717955-0.056570851*x)*x)*x)*x;
    // printf("%.20f\n", 1-a);
    return -1.7417939+(2.8212026+(-1.4699568+(0.44717955-0.056570851*x)*x)*x)*x+0.6931471806*exponent;
    // return -1.7417939+(2.8212026+(-1.4699568+(0.44717955-0.056570851*x)*x)*x)*x+0.6930916*exponent;
}

float h1_maxf(float a, float b) {return a > b ? a : b;}
int   h1_maxi(int a, int b) {return a > b ? a : b;}
float h1_minf(float a, float b) {return a < b ? a : b;}
int   h1_mini(int a, int b) {return a < b ? a : b;}

float h1_nanf() {
    int n = 0x7fc00000;
    return *(float*)&n;
}

/** Accurate to around 4 significant digits. Pretty good! */
float h1_powf(float a, float b) {
    return exp(b * ln(a));
}

float h1_powfi(float a, int b) {
    if (b < 0) { return 0; }
    if (b == 0) { return 1; }
    int tmp = h1_powfi(a, b/2);
    if (b % 2 == 1) {
        return tmp * tmp * a;
    }
    else {
        return tmp * tmp;
    }
}

int h1_powii(int a, int b) {
    if (b < 0) { return 0; }
    if (b == 0) { return 1; }
    int tmp = h1_powii(a, b/2);
    if (b % 2 == 1) {
        return tmp * tmp * a;
    }
    else {
        return tmp * tmp;
    }
}

int h1_roundf(float f) {
    float res = f - (int)f;
    int s = sgn(f);
    if (absf(res) >= 0.5) { return (int)f + s; }
    else { return (int)f; }
}

int h1_sgnf(float f) {return f > 0 ? 1 : f == 0 ? 0 : -1;}

float h1_sqrtf(float f) {
    if (f < 0) { return -nan; }

    // for f = a * 2^2n, find a and n with funny bit fiddling
    int bits = *(int*)&f;
    int exponent = (bits >> 23) - 127;
    int mantissa = (bits & 0x007fffff) | 0x3f800000;
    float a = *(float*)&mantissa;

    if (exponent % 2 == 1) {
        a /= 2;
        exponent++;
    }
    int n = exponent / 2;
    // initial guess as least squares regression
    float guess = (0.485 * a + 0.485) * (1 << n);

    // heron's method
    float eps = 1e-6;
    float old_guess;
    do {
        old_guess = guess;
        guess = 0.5 * ((f / guess) + guess);
    } while (absf(guess - old_guess) > eps);

    return guess;
}

#endif /* #ifndef IMPL_MATHS_H_DONE */
#endif /* #ifdef IMPL_MATHS_H */
