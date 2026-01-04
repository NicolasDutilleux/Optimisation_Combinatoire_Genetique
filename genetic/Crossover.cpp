#include "genetic/Crossover.h"
#include "utils/Random.h"
#include <algorithm>
#include <unordered_set>

std::vector<int> Order_Crossover(const std::vector<int>& parentA, const std::vector<int>& parentB)
{
    if (parentA.size() != parentB.size()) return {};
    int n = static_cast<int>(parentA.size());
    std::vector<int> child(n, -1);

    int cut1 = RandInt(1, n - 2);
    int cut2 = RandInt(cut1 + 1, n - 1);

    // OPTIMIZED: Use unordered_set for O(1) membership checking
    std::unordered_set<int> child_set;
    for (int i = cut1; i <= cut2; ++i)
    {
        child[i] = parentA[i];
        child_set.insert(parentA[i]);
    }

    int idx = (cut2 + 1) % n;
    for (int i = 0; i < n; ++i)
    {
        int candidate = parentB[(cut2 + 1 + i) % n];
        if (child_set.count(candidate) == 0)
        {
            child[idx] = candidate;
            child_set.insert(candidate);
            idx = (idx + 1) % n;
        }
    }

    // Fill remaining -1 positions with missing genes
    std::unordered_set<int> all_genes;
    for (int i = 1; i <= n; ++i)
        all_genes.insert(i);
    
    for (int id : child_set)
        all_genes.erase(id);

    int fill_idx = 0;
    for (int i = 0; i < n; ++i)
    {
        if (child[i] == -1)
        {
            auto it = all_genes.begin();
            if (it != all_genes.end())
            {
                child[i] = *it;
                all_genes.erase(it);
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

std::vector<int> Slice_Crossover(
    const std::vector<int>& parentA,
    const std::vector<int>& parentB)
{
    int m = static_cast<int>(parentA.size());
    if (m == 0) return parentA;
    if (m == 1) return parentA;

    int max_id = 0;
    for (int id : parentA) if (id > max_id) max_id = id;
    for (int id : parentB) if (id > max_id) max_id = id;

    if (max_id <= 0) return parentA;

    int cut1 = RandInt(0, m - 1);
    int cut2 = RandInt(0, m - 1);
    if (cut1 > cut2) std::swap(cut1, cut2);

    std::vector<int> child;
    std::vector<bool> used(max_id + 1, false);

    for (int i = cut1; i <= cut2; ++i)
    {
        child.push_back(parentA[i]);
        if (parentA[i] > 0 && parentA[i] <= max_id)
            used[parentA[i]] = true;
    }

    for (int id : parentB)
    {
        if (id > 0 && id <= max_id && !used[id])
        {
            child.push_back(id);
            used[id] = true;
        }
    }

    return child;
}

