#include "utils/Random.h"

std::mt19937& GLOBAL_RNG()
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    return g;
}

int RandInt(int a, int b)
{
    std::uniform_int_distribution<int> d(a, b);
    return d(GLOBAL_RNG());
}

double RandDouble()
{
    std::uniform_real_distribution<double> d(0.0, 1.0);
    return d(GLOBAL_RNG());
}
