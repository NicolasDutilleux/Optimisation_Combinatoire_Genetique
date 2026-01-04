#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <vector>

struct Individual {
    std::vector<int> active_ring;  // IDs of active stations forming the ring (1..N)
};

#endif