#include "genetic/Crossover.h"
#include "utils/Random.h"
#include <algorithm>
#include <vector>
#include <unordered_set>

// On remplace la logique Slice par celle du code simple (Order Crossover)
std::vector<int> Slice_Crossover(const std::vector<int>& p1, const std::vector<int>& p2)
{
    int size_p1 = static_cast<int>(p1.size());
    if (size_p1 < 2) return p1;

    // 1. Choisir deux points de coupure aléatoires
    int cut1 = RandInt(0, size_p1 - 1);
    int cut2 = RandInt(0, size_p1 - 1);
    if (cut1 > cut2) std::swap(cut1, cut2);

    // 2. Initialiser l'enfant
    std::vector<int> child;
    child.reserve(size_p1);

    // 3. Copier le segment du parent 1 (Héritage de la structure locale)
    // On utilise un Set pour savoir rapidement ce qu'on a déjà mis
    std::unordered_set<int> added_genes;
    for (int i = cut1; i <= cut2; ++i) {
        child.push_back(p1[i]);
        added_genes.insert(p1[i]);
    }

    // 4. Compléter avec les gènes du parent 2 (dans l'ordre où ils apparaissent)
    // C'est ça qui "fonctionne mieux" que le slice complexe
    for (int gene : p2) {
        if (added_genes.find(gene) == added_genes.end()) {
            child.push_back(gene);
            added_genes.insert(gene);
        }
    }

    return child;
}