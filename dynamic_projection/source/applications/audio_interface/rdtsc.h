#include "stdint.h"
inline uint64_t rdtsc(void)
{
  uint64_t result=0;
        unsigned a, d;

        do {
                __asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d));
                result = ((uint64_t)a) | (((uint64_t)d) << 32);
        } while (__builtin_expect ((int) result == -1, 0));
        return result;
}
