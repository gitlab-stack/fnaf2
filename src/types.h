/*
 * types.h - Portable type definitions, fixed-point math, and RNG
 *
 * Five Nights at Freddy's 2 - C Port
 * Cross-platform game engine designed for maximum portability.
 * Targets: PC (SDL2), PS1, Xbox 360, and more.
 */

#ifndef FNAF2_TYPES_H
#define FNAF2_TYPES_H

/* ---- Standard integer types ---- */
/* If your platform lacks <stdint.h>, provide typedefs here */
#ifdef FNAF2_NO_STDINT
    typedef signed char             int8_t;
    typedef unsigned char           uint8_t;
    typedef signed short            int16_t;
    typedef unsigned short          uint16_t;
    typedef signed int              int32_t;
    typedef unsigned int            uint32_t;
    typedef signed long long        int64_t;
    typedef unsigned long long      uint64_t;
    typedef unsigned int            size_t;
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

/* ---- Boolean type ---- */
#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined)
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
        #include <stdbool.h>
    #else
        typedef int bool;
        #define true  1
        #define false 0
    #endif
#endif

/* ---- NULL ---- */
#ifndef NULL
    #define NULL ((void *)0)
#endif

/* ---- Fixed-point math (16.16) ---- */
/* Use this for platforms without FPU (e.g., PS1) */
typedef int32_t fixed_t;

#define FIXED_SHIFT         16
#define FIXED_ONE           (1 << FIXED_SHIFT)
#define FIXED_HALF          (1 << (FIXED_SHIFT - 1))

#define INT_TO_FIXED(x)    ((fixed_t)((x) << FIXED_SHIFT))
#define FIXED_TO_INT(x)    ((int)((x) >> FIXED_SHIFT))

#ifndef FNAF2_NO_FLOAT
    #define FLOAT_TO_FIXED(x)  ((fixed_t)((x) * FIXED_ONE))
    #define FIXED_TO_FLOAT(x)  ((float)(x) / FIXED_ONE)
#endif

#define FIXED_MUL(a, b)    ((fixed_t)(((int64_t)(a) * (b)) >> FIXED_SHIFT))
#define FIXED_DIV(a, b)    ((fixed_t)(((int64_t)(a) << FIXED_SHIFT) / (b)))
#define FIXED_ABS(x)       ((x) < 0 ? -(x) : (x))

/* ---- Portable random number generator (LCG) ---- */
typedef struct {
    uint32_t state;
} RNG;

static inline void rng_seed(RNG *rng, uint32_t seed)
{
    rng->state = seed ? seed : 1;
}

static inline uint32_t rng_next(RNG *rng)
{
    rng->state = rng->state * 1103515245u + 12345u;
    return (rng->state >> 16) & 0x7FFF;
}

/* Returns random number in range [0, max) */
static inline uint32_t rng_range(RNG *rng, uint32_t max)
{
    if (max == 0) return 0;
    return rng_next(rng) % max;
}

/* Returns random number in range [min, max] (inclusive) */
static inline uint32_t rng_range_inclusive(RNG *rng, uint32_t min, uint32_t max)
{
    if (max <= min) return min;
    return min + (rng_next(rng) % (max - min + 1));
}

/* ---- Utility macros ---- */
#define FNAF2_MIN(a, b)         ((a) < (b) ? (a) : (b))
#define FNAF2_MAX(a, b)         ((a) > (b) ? (a) : (b))
#define FNAF2_CLAMP(x, lo, hi) FNAF2_MAX((lo), FNAF2_MIN((hi), (x)))
#define FNAF2_ARRAY_COUNT(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define FNAF2_UNUSED(x)         ((void)(x))

/* ---- Memory helpers ---- */
/* Simple memset/memcpy for platforms without <string.h> */
#ifdef FNAF2_NO_STDLIB
    static inline void fnaf2_memset(void *dst, int val, size_t n)
    {
        unsigned char *d = (unsigned char *)dst;
        while (n--) *d++ = (unsigned char)val;
    }
    static inline void fnaf2_memcpy(void *dst, const void *src, size_t n)
    {
        unsigned char *d = (unsigned char *)dst;
        const unsigned char *s = (const unsigned char *)src;
        while (n--) *d++ = *s++;
    }
    #define FNAF2_MEMSET fnaf2_memset
    #define FNAF2_MEMCPY fnaf2_memcpy
#else
    #include <string.h>
    #define FNAF2_MEMSET memset
    #define FNAF2_MEMCPY memcpy
#endif

#endif /* FNAF2_TYPES_H */
