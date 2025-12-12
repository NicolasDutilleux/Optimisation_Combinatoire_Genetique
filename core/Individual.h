#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <vector>

struct Individual {
    std::vector<int> ids;      // permutation (1..N) Ids
    std::vector<bool> mask;     // visit mask
};

#endif

