// utils/Random.c - Pure C implementation
#include "Random.h"
#include <stdlib.h>
#include <time.h>

void RNG_Init(void)
{
    // no global seed required when using rand_s on MSVC
    (void)time(NULL);
}

int RandInt(int a, int b)
{
    if (a > b) {
        int tmp = a; a = b; b = tmp;
    }
#ifdef _MSC_VER
    unsigned int v;
    rand_s(&v);
    return a + (int)(v % (unsigned int)(b - a + 1));
#else
    return a + (rand() % (b - a + 1));
#endif
}

double RandDouble(void)
{
#ifdef _MSC_VER
    unsigned int v;
    rand_s(&v);
    return (double)v / (double)UINT_MAX;
#else
    return (double)rand() / (double)RAND_MAX;
#endif
}
