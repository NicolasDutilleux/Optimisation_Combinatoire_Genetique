// utils/Visualize.c - Ring visualization using HTML/SVG
#define _CRT_SECURE_NO_WARNINGS

#include "Visualize.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <direct.h>  // For _mkdir
#include <windows.h>

void Visualize_Ring(const Individual* ind, const Node* nodes, int total_stations,
                    double cost, int alpha, int generation)
{
    if (!ind || !nodes || ind->ring_size < 2) return;

    // Create images directory if it doesn't exist
    _mkdir("images");

    // Create HTML file in images folder
    char filename[256];
    sprintf(filename, "images\\ring_gen%d_alpha%d_cost%.0f.html", generation, alpha, cost);
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot create visualization file\n");
        return;
    }

    // Find min/max coordinates for scaling
    double min_x = DBL_MAX, max_x = -DBL_MAX;
    double min_y = DBL_MAX, max_y = -DBL_MAX;
    
    for (int i = 0; i < total_stations; i++) {
        if (nodes[i].x < min_x) min_x = nodes[i].x;
        if (nodes[i].x > max_x) max_x = nodes[i].x;
        if (nodes[i].y < min_y) min_y = nodes[i].y;
        if (nodes[i].y > max_y) max_y = nodes[i].y;
    }

    // Add margin
    double margin = 50;
    double width = 900;
    double height = 600;
    double scale_x = (width - 2 * margin) / (max_x - min_x + 1);
    double scale_y = (height - 2 * margin) / (max_y - min_y + 1);
    double scale = (scale_x < scale_y) ? scale_x : scale_y;

    // Create set of nodes in ring for quick lookup
    int* in_ring = (int*)calloc(total_stations + 1, sizeof(int));
    for (int i = 0; i < ind->ring_size; i++) {
        if (ind->active_ring[i] > 0 && ind->active_ring[i] <= total_stations) {
            in_ring[ind->active_ring[i]] = 1;
        }
    }

    // Compute assignments (non-ring node -> nearest ring node)
    int* assignments = (int*)calloc(total_stations + 1, sizeof(int));
    for (int i = 0; i < total_stations; i++) {
        int node_id = nodes[i].id;
        if (in_ring[node_id]) {
            assignments[node_id] = node_id;  // Ring node assigned to itself
            continue;
        }
        
        // Find nearest ring node
        double best_dist = DBL_MAX;
        int best_ring_node = -1;
        
        for (int j = 0; j < ind->ring_size; j++) {
            int ring_id = ind->active_ring[j];
            if (ring_id <= 0 || ring_id > total_stations) continue;
            
            double dx = nodes[i].x - nodes[ring_id - 1].x;
            double dy = nodes[i].y - nodes[ring_id - 1].y;
            double d = dx * dx + dy * dy;
            
            if (d < best_dist) {
                best_dist = d;
                best_ring_node = ring_id;
            }
        }
        assignments[node_id] = best_ring_node;
    }

    // Write HTML header
    fprintf(f, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(f, "<title>Ring Visualization - Generation %d</title>\n", generation);
    fprintf(f, "<style>\n");
    fprintf(f, "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }\n");
    fprintf(f, ".container { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); max-width: 1200px; }\n");
    fprintf(f, "h1 { color: #333; }\n");
    fprintf(f, ".info { margin: 10px 0; padding: 10px; background: #e8f4e8; border-radius: 5px; }\n");
    fprintf(f, ".legend { display: flex; gap: 20px; margin: 10px 0; flex-wrap: wrap; }\n");
    fprintf(f, ".legend-item { display: flex; align-items: center; gap: 5px; }\n");
    fprintf(f, ".legend-color { width: 20px; height: 20px; border-radius: 50%%; }\n");
    fprintf(f, ".two-columns { display: flex; gap: 20px; flex-wrap: wrap; }\n");
    fprintf(f, ".column { flex: 1; min-width: 300px; }\n");
    fprintf(f, ".assignments { max-height: 400px; overflow-y: auto; background: #f9f9f9; padding: 10px; border-radius: 5px; font-family: monospace; font-size: 12px; }\n");
    fprintf(f, ".assignment-item { padding: 2px 0; }\n");
    fprintf(f, ".ring-node { color: #2196F3; font-weight: bold; }\n");
    fprintf(f, ".non-ring-node { color: #666; }\n");
    fprintf(f, "</style>\n</head>\n<body>\n");
    
    fprintf(f, "<div class='container'>\n");
    fprintf(f, "<h1>Ring Optimization - Generation %d</h1>\n", generation);
    fprintf(f, "<div class='info'>\n");
    fprintf(f, "<strong>Cost:</strong> %.2f | ", cost);
    fprintf(f, "<strong>Alpha:</strong> %d | ", alpha);
    fprintf(f, "<strong>Ring size:</strong> %d / %d stations | ", ind->ring_size, total_stations);
    fprintf(f, "<strong>Non-ring:</strong> %d stations\n", total_stations - ind->ring_size);
    fprintf(f, "</div>\n");
    
    fprintf(f, "<div class='legend'>\n");
    fprintf(f, "<div class='legend-item'><div class='legend-color' style='background:#2196F3'></div> Ring nodes</div>\n");
    fprintf(f, "<div class='legend-item'><div class='legend-color' style='background:#9E9E9E'></div> Non-ring nodes</div>\n");
    fprintf(f, "<div class='legend-item'><div class='legend-color' style='background:#4CAF50'></div> Depot (node 1)</div>\n");
    fprintf(f, "<div class='legend-item'><span style='color:#2196F3'>---</span> Ring edges</div>\n");
    fprintf(f, "<div class='legend-item'><span style='color:#BDBDBD'>- - -</span> Assignments</div>\n");
    fprintf(f, "</div>\n");

    fprintf(f, "<div class='two-columns'>\n");
    fprintf(f, "<div class='column'>\n");

    // SVG
    fprintf(f, "<svg width='%.0f' height='%.0f' style='border: 1px solid #ccc; background: white;'>\n", width, height);

    // Draw connections from non-ring nodes to nearest ring node (light gray dashed)
    for (int i = 0; i < total_stations; i++) {
        int node_id = nodes[i].id;
        if (in_ring[node_id]) continue;
        
        int best_ring_node = assignments[node_id];
        
        if (best_ring_node > 0) {
            double x1 = margin + (nodes[i].x - min_x) * scale;
            double y1 = height - margin - (nodes[i].y - min_y) * scale;
            double x2 = margin + (nodes[best_ring_node - 1].x - min_x) * scale;
            double y2 = height - margin - (nodes[best_ring_node - 1].y - min_y) * scale;
            
            fprintf(f, "<line x1='%.1f' y1='%.1f' x2='%.1f' y2='%.1f' stroke='#BDBDBD' stroke-width='1' stroke-dasharray='4,4'/>\n",
                    x1, y1, x2, y2);
        }
    }

    // Draw ring edges (blue)
    for (int i = 0; i < ind->ring_size; i++) {
        int id1 = ind->active_ring[i];
        int id2 = ind->active_ring[(i + 1) % ind->ring_size];
        
        if (id1 <= 0 || id1 > total_stations || id2 <= 0 || id2 > total_stations) continue;
        
        double x1 = margin + (nodes[id1 - 1].x - min_x) * scale;
        double y1 = height - margin - (nodes[id1 - 1].y - min_y) * scale;
        double x2 = margin + (nodes[id2 - 1].x - min_x) * scale;
        double y2 = height - margin - (nodes[id2 - 1].y - min_y) * scale;
        
        fprintf(f, "<line x1='%.1f' y1='%.1f' x2='%.1f' y2='%.1f' stroke='#2196F3' stroke-width='2'/>\n",
                x1, y1, x2, y2);
    }

    // Draw all nodes
    for (int i = 0; i < total_stations; i++) {
        double x = margin + (nodes[i].x - min_x) * scale;
        double y = height - margin - (nodes[i].y - min_y) * scale;
        int node_id = nodes[i].id;
        
        const char* color;
        int radius;
        
        if (node_id == 1) {
            color = "#4CAF50";  // Green for depot
            radius = 8;
        } else if (in_ring[node_id]) {
            color = "#2196F3";  // Blue for ring nodes
            radius = 6;
        } else {
            color = "#9E9E9E";  // Gray for non-ring nodes
            radius = 4;
        }
        
        fprintf(f, "<circle cx='%.1f' cy='%.1f' r='%d' fill='%s' stroke='white' stroke-width='1'>\n",
                x, y, radius, color);
        fprintf(f, "<title>Node %d (%.1f, %.1f)%s</title>\n", 
                node_id, nodes[i].x, nodes[i].y,
                in_ring[node_id] ? " [RING]" : "");
        fprintf(f, "</circle>\n");
    }

    // Draw node labels for ring nodes
    for (int i = 0; i < ind->ring_size; i++) {
        int id = ind->active_ring[i];
        if (id <= 0 || id > total_stations) continue;
        
        double x = margin + (nodes[id - 1].x - min_x) * scale;
        double y = height - margin - (nodes[id - 1].y - min_y) * scale;
        
        fprintf(f, "<text x='%.1f' y='%.1f' font-size='10' fill='#333' text-anchor='middle' dy='-10'>%d</text>\n",
                x, y, id);
    }

    fprintf(f, "</svg>\n");
    fprintf(f, "</div>\n");  // End column 1

    // Column 2: Assignments list
    fprintf(f, "<div class='column'>\n");
    fprintf(f, "<h3>Assignments</h3>\n");
    fprintf(f, "<div class='assignments'>\n");
    
    // Ring nodes first
    fprintf(f, "<strong>Ring nodes (%d):</strong><br>\n", ind->ring_size);
    for (int i = 0; i < ind->ring_size; i++) {
        int id = ind->active_ring[i];
        fprintf(f, "<span class='ring-node'>%d</span>", id);
        if (i < ind->ring_size - 1) fprintf(f, " &rarr; ");
    }
    fprintf(f, " &rarr; <span class='ring-node'>%d</span> (loop)<br><br>\n", ind->active_ring[0]);
    
    // Non-ring assignments
    fprintf(f, "<strong>Non-ring assignments (%d):</strong><br>\n", total_stations - ind->ring_size);
    for (int i = 1; i <= total_stations; i++) {
        if (!in_ring[i]) {
            fprintf(f, "<div class='assignment-item'><span class='non-ring-node'>%d</span> &rarr; <span class='ring-node'>%d</span></div>\n",
                    i, assignments[i]);
        }
    }
    
    fprintf(f, "</div>\n");  // End assignments
    fprintf(f, "</div>\n");  // End column 2
    fprintf(f, "</div>\n");  // End two-columns
    
    fprintf(f, "</div>\n</body>\n</html>\n");
    
    fclose(f);
    free(in_ring);
    free(assignments);

    printf("[VISUALIZATION] Saved to %s\n", filename);
    
    // Open in default browser
    char full_path[512];
    GetFullPathNameA(filename, sizeof(full_path), full_path, NULL);
    ShellExecuteA(NULL, "open", full_path, NULL, NULL, SW_SHOWNORMAL);
}
