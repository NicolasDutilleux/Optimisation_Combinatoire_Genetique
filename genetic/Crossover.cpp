#include "genetic/Crossover.h"
#include "utils/Random.h"

#include <algorithm>

std::vector<int> Order_Crossover(const std::vector<int>& parentA, const std::vector<int>& parentB)
{
    if (parentA.size() != parentB.size()) return {};
    int n = static_cast<int>(parentA.size());
    std::vector<int> child(n, -1);

    int cut1 = RandInt(1, n - 2);
    int cut2 = RandInt(cut1 + 1, n - 1);

    for (int i = cut1; i <= cut2; ++i)
        child[i] = parentA[i];

    int idx = (cut2 + 1) % n;
    for (int i = 0; i < n; ++i)
    {
        int candidate = parentB[(cut2 + 1 + i) % n];
        bool present = false;
        for (int x = 0; x < n; ++x) if (child[x] == candidate) { present = true; break; }

        if (!present)
        {
            child[idx] = candidate;
            idx = (idx + 1) % n;
        }
    }

    for (int i = 0; i < n; ++i)
    {
        if (child[i] == -1)
        {
            for (int g = 1; g <= n; ++g)
            {
                bool found = false;
                for (int x = 0; x < n; ++x) if (child[x] == g) { found = true; break; }
                if (!found) { child[i] = g; break; }
            }
        }
    }
    return child;
}

std::vector<bool> Mask_Crossover(const std::vector<bool>& m1,
    const std::vector<bool>& m2,
    int min_active)
{
    int n = static_cast<int>(m1.size());
    if ((int)m2.size() != n) return m1;

    std::vector<bool> child(n, false);

    for (int i = 0; i < n; ++i)
        child[i] = (m1[i] == m2[i]) ? m1[i] : (RandDouble() < 0.5);

    if (n > 0) child[0] = true;

    int active = 0;
    for (bool b : child) if (b) ++active;

    if (active < min_active)
    {
        std::vector<int> candidates;
        for (int i = 1; i < n; ++i) if (m1[i] || m2[i]) candidates.push_back(i);
        if (candidates.empty()) for (int i = 1; i < n; ++i) candidates.push_back(i);

        while (active < min_active && !candidates.empty())
        {
            int idx = RandInt(0, (int)candidates.size() - 1);
            int pos = candidates[idx];
            if (!child[pos]) { child[pos] = true; ++active; }
            candidates[idx] = candidates.back();
            candidates.pop_back();
        }
    }
    return child;
}

