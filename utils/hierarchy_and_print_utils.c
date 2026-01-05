#include "hierarchy_and_print_utils.h"
#include <stdio.h>
#include <stdlib.h>

// C implementations for assignment pairs and SVG plotting
int* BuildAssignmentPairs(const int* active_ring, int ring_size, const int** ranking, int total_stations, int* out_len)
{
    if (!active_ring || ring_size <= 0 || !ranking || total_stations <= 0) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    int* is_active = (int*)calloc(total_stations + 1, sizeof(int));
    if (!is_active) { if (out_len) *out_len = 0; return NULL; }
    for (int i = 0; i < ring_size; ++i) {
        int id = active_ring[i];
        if (id >= 1 && id <= total_stations) is_active[id] = 1;
    }

    int* pairs = (int*)malloc(2 * total_stations * sizeof(int));
    if (!pairs) { free(is_active); if (out_len) *out_len = 0; return NULL; }
    int p = 0;

    for (int s = 1; s <= total_stations; ++s) {
        if (!is_active[s]) {
            const int* row = ranking[s - 1];
            int nearest = -1;
            for (int j = 0; j < total_stations; ++j) {
                int cand = row[j];
                if (cand >= 1 && cand <= total_stations && is_active[cand]) { nearest = cand; break; }
            }
            if (nearest > 0) {
                pairs[p++] = s;
                pairs[p++] = nearest;
            }
        }
    }

    free(is_active);
    if (out_len) *out_len = p;
    if (p == 0) { free(pairs); return NULL; }
    return pairs;
}

void PlotIndividualSVG_C(const int* active_ring, int ring_size, const Node* node_vector, int num_nodes,
                         int generation, const int* assignment_pairs, int assignment_len)
{
    if (!active_ring || ring_size <= 0 || !node_vector || num_nodes <= 0) return;

#ifdef _WIN32
    system("mkdir Images 2>nul");
#else
    system("mkdir -p Images 2>/dev/null");
#endif

    int max_id = 0;
    for (int i = 0; i < num_nodes; ++i) if (node_vector[i].id > max_id) max_id = node_vector[i].id;
    int map_size = (max_id > num_nodes) ? max_id + 1 : num_nodes + 1;
    int* id_to_index = (int*)malloc(map_size * sizeof(int));
    if (!id_to_index) return;
    for (int i = 0; i < map_size; ++i) id_to_index[i] = -1;
    for (int i = 0; i < num_nodes; ++i) {
        int id = node_vector[i].id;
        if (id >= 0 && id < map_size) id_to_index[id] = i;
    }

    int minX = node_vector[0].x, maxX = node_vector[0].x;
    int minY = node_vector[0].y, maxY = node_vector[0].y;
    for (int i = 0; i < num_nodes; ++i) {
        if (node_vector[i].x < minX) minX = node_vector[i].x;
        if (node_vector[i].x > maxX) maxX = node_vector[i].x;
        if (node_vector[i].y < minY) minY = node_vector[i].y;
        if (node_vector[i].y > maxY) maxY = node_vector[i].y;
    }
    double rangeX = maxX - minX; if (rangeX == 0) rangeX = 1.0;
    double rangeY = maxY - minY; if (rangeY == 0) rangeY = 1.0;
    double scale = 900.0 / (rangeX > rangeY ? rangeX : rangeY);
    double offset = 50.0;

    char filename[256];
    snprintf(filename, sizeof(filename), "Images/plot_individual_%d.svg", generation);
    FILE* out = fopen(filename, "w");
    if (!out) { free(id_to_index); return; }

    fprintf(out, "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n");
    fprintf(out, "<rect width='100%%' height='100%%' fill='white'/>\n");

    for (int i = 0; i < num_nodes; ++i) {
        double sx = (node_vector[i].x - minX) * scale + offset;
        double sy = (node_vector[i].y - minY) * scale + offset;
        fprintf(out, "<circle cx='%.2f' cy='%.2f' r='3' fill='blue'/>\n", sx, sy);
    }

    if (ring_size > 0) {
        // Draw links using the individual's tour order (active_ring order)
        // Print the individual ids being connected (in tour order)
        printf("[PLOT] Drawing red links for individual (tour order): ");
        for (int zz = 0; zz < ring_size; ++zz) printf("%d ", active_ring[zz]);
        printf("\n");

        for (int i = 0; i + 1 < ring_size; ++i) {
            int id_a = active_ring[i];
            int id_b = active_ring[i+1];
            int a = (id_a >= 0 && id_a < map_size) ? id_to_index[id_a] : -1;
            int b = (id_b >= 0 && id_b < map_size) ? id_to_index[id_b] : -1;
            if (a >= 0 && a < num_nodes && b >= 0 && b < num_nodes) {
                double ax = (node_vector[a].x - minX) * scale + offset;
                double ay = (node_vector[a].y - minY) * scale + offset;
                double bx = (node_vector[b].x - minX) * scale + offset;
                double by = (node_vector[b].y - minY) * scale + offset;
                fprintf(out, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='3'/>\n", ax, ay, bx, by);
            }
        }
        // close loop: last -> first
        if (ring_size >= 2) {
            int id_a = active_ring[ring_size - 1];
            int id_b = active_ring[0];
            int a = (id_a >= 0 && id_a < map_size) ? id_to_index[id_a] : -1;
            int b = (id_b >= 0 && id_b < map_size) ? id_to_index[id_b] : -1;
            if (a >= 0 && a < num_nodes && b >= 0 && b < num_nodes) {
                double ax = (node_vector[a].x - minX) * scale + offset;
                double ay = (node_vector[a].y - minY) * scale + offset;
                double bx = (node_vector[b].x - minX) * scale + offset;
                double by = (node_vector[b].y - minY) * scale + offset;
                fprintf(out, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='3'/>\n", ax, ay, bx, by);
            }
        }
    }

    for (int i = 0; i + 1 < assignment_len; i += 2) {
        int id_a = assignment_pairs[i];
        int id_b = assignment_pairs[i+1];
        int a = (id_a >=0 && id_a < map_size) ? id_to_index[id_a] : -1;
        int b = (id_b >=0 && id_b < map_size) ? id_to_index[id_b] : -1;
        if (a>=0 && a<num_nodes && b>=0 && b<num_nodes) {
            double ax = (node_vector[a].x - minX) * scale + offset;
            double ay = (node_vector[a].y - minY) * scale + offset;
            double bx = (node_vector[b].x - minX) * scale + offset;
            double by = (node_vector[b].y - minY) * scale + offset;
            fprintf(out, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='blue' stroke-width='0.5'/>\n", ax, ay, bx, by);
        }
    }

    for (int i = 0; i < ring_size; ++i) {
        int id = active_ring[i];
        int idx = (id>=0 && id < map_size) ? id_to_index[id] : -1;
        if (idx>=0 && idx<num_nodes) {
            double sx = (node_vector[idx].x - minX) * scale + offset;
            double sy = (node_vector[idx].y - minY) * scale + offset;
            if (active_ring[i] == 1) fprintf(out, "<circle cx='%.2f' cy='%.2f' r='8' fill='darkblue'/>\n", sx, sy);
            else fprintf(out, "<circle cx='%.2f' cy='%.2f' r='6' fill='red'/>\n", sx, sy);
        }
    }

    fprintf(out, "</svg>\n");
    fclose(out);
    free(id_to_index);
}