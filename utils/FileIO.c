#define _CRT_SECURE_NO_WARNINGS
#include "FileIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* readDataset(const char* filename, int* out_count)
{
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;

    char line[256];
    // Skip until BEGIN
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "BEGIN")) break;
    }

    int capacity = 256;
    Node* nodes = (Node*)malloc(capacity * sizeof(Node));
    if (!nodes) { fclose(f); return NULL; }

    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "END")) break;
        int id, x, y;
        if (sscanf(line, "%d %d %d", &id, &x, &y) == 3) {
            if (count >= capacity) {
                capacity *= 2;
                Node* tmp = (Node*)realloc(nodes, capacity * sizeof(Node));
                if (!tmp) { free(nodes); fclose(f); return NULL; }
                nodes = tmp;
            }
            nodes[count].id = id;
            nodes[count].x = x;
            nodes[count].y = y;
            count++;
        }
    }

    fclose(f);
    *out_count = count;
    return nodes;
}
