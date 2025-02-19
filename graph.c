#pragma once

typedef struct edge_t edge_t;
struct edge_t{
    int dest;        // ID de la station destination
    double weight;   // Poids de l'arête (par exemple, durée du trajet)
    edge_t *next;    // Pointeur vers l'arête suivante dans la liste
};

typedef struct {
    char *stop_id;          // ID de la station (stop_id)
    char *name;      // Nom de la station
    edge_t *edges;     // Liste d'arêtes pour cette station
} node_t;

typedef struct Graph {
    unsigned int numNodes; // Nombre de stations (noeuds)
    node_t *nodes;         // Tableau des noeuds (stations)
} graph_t;
