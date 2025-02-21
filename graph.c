#pragma once

typedef struct edge_t edge_t;
typedef struct node_t node_t;
typedef struct graph_t graph_t;

typedef enum { TRAIN, BUS, METRO } TransportType;

struct edge_t {             // = trajet
    char *trip_id;          // trip_id du trajet
    double weight;          // Poids/durée du trajet
    TransportType type;     // Metro, bus, train
    node_t *to;             // pointeur vers la station suivante
    unsigned int *horaires  // Liste des horaires de départ [9:00, 9:30, ...] en secondes depuis minuit (ex: 36000 pour 10:00)
};

struct node_t {         // = station
    char *stop_id;      // Stop_ID de la station
    char *name;         // Nom de la station
    size_t numEdges;    // Nombre de trajets
    edge_t *edges;      // Liste de trajets pour cette station
};

struct graph_t {
    size_t numNodes;    // Nombre de stations (noeuds)
    node_t *nodes;      // Tableau des noeuds (stations)
};

node_t *add_node(graph_t *graph, const char *stop_id) {
    // Chercher si ce noeud existe déjà dans le graphe
    // Si non, ajouter un nouveau noeud avec le stop_id donné
}

void add_edge(node_t *node_from, const char *stop_id_to, double weight) {
    // Créer une arête entre node_from et stop_id_to avec une certaine durée/poids
    edge_t *edge = malloc(sizeof(edge_t));
    edge->stop_id_dest = strdup(stop_id_to);
    edge->weight = weight;
    edge->next = node_from->edges;
    node_from->edges = edge;
}


// pseudo-code
void build_graph() {
    mmap(stops.txt);
    mmap(trips.txt);

    for (station in stops.txt) {
        add_node(graph, station);

        trip_list[] = NULL;
        trip_number = search(station in trips.txt);
        if (trip_number == 0) continue; // cet station n'a aucun trajet
        trip_list = malloc(trip_number);

        for (trip in trip_list) {
            add_edge(station, trip);
        }
    }
}
