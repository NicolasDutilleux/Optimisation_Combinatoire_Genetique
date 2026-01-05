#ifndef FILEIO_H
#define FILEIO_H

#include "core\Node.h"

// Read dataset file; returns malloc'd array of Node (caller must free)
Node* readDataset(const char* filename, int* out_count);

#endif
