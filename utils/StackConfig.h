// utils/StackConfig.h
// Configuration globale pour l'optimisation Stack vs Heap
//
// Ce fichier définit le seuil unique pour décider quand utiliser
// l'allocation stack (rapide) vs heap (flexible) dans tout le projet.

#ifndef STACK_CONFIG_H
#define STACK_CONFIG_H

// =============================================================================
// SEUIL GLOBAL : Stack vs Heap
// =============================================================================
//
// Utilisation :
//   - Si (taille_nécessaire <= MAX_STACK_BUFFER_SIZE) ? Stack (rapide)
//   - Si (taille_nécessaire > MAX_STACK_BUFFER_SIZE)  ? Heap (flexible)
//
// Valeur choisie : 512
// Raison :
//   - Suffisant pour tous les datasets actuels (51, 100, 127, 225 stations)
//   - Taille stack : 512 × 4 bytes = 2 KB (acceptable, stack totale ~1-8 MB)
//   - Évite stack overflow même pour problèmes moyens (<512 stations)
//
// Note historique :
//   - Auparavant : MAX_STACK_STATIONS (512) dans Cost.c
//   - Auparavant : MAX_STACK_SIZE (256) dans Crossover.c et Mutation.c
//   - Unifié pour cohérence et maintenabilité
//
#define MAX_STACK_BUFFER_SIZE 512

// =============================================================================
// UTILISATION DANS LE CODE
// =============================================================================
//
// Exemple type (pattern utilisé dans Cost.c, Crossover.c, Mutation.c) :
//
//   int stack_buffer[MAX_STACK_BUFFER_SIZE];
//   int* array;
//   int use_heap = (required_size > MAX_STACK_BUFFER_SIZE);
//   
//   if (use_heap) {
//       array = (int*)calloc(required_size, sizeof(int));
//       if (!array) return ERROR;
//   } else {
//       array = stack_buffer;
//       memset(array, 0, required_size * sizeof(int));
//   }
//   
//   // ... Utilisation de array ...
//   
//   if (use_heap) free(array);
//
// =============================================================================

#endif // STACK_CONFIG_H
