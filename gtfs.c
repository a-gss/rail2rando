#pragma once

enum gtfs_index {
    calendar,
    calendar_dates,
    routes,
    stop_times,
    stops,
    trips,
    GTFS_FILE_NUMBER
};

const char *gtfs_filepath[] = {
    "GTFS/LIO/calendar.txt",
    "GTFS/LIO/calendar_dates.txt",
    "GTFS/LIO/routes.txt",
    "GTFS/LIO/stop_times.txt",
    "GTFS/LIO/stops.txt",
    "GTFS/LIO/trips.txt",
};

// TODO: optimize struct packing
// gcc already optimize it, but let's do it anyway

typedef struct gtfs_file_t gtfs_file_t;
typedef struct trip_t trip_t;
typedef struct stop_t stop_t;
typedef struct graph_t graph_t;
typedef enum { TRAIN, BUS, METRO } TransportType;

struct gtfs_file_t {
    const char *filepath;
    char *data;
    off_t filesize;
};

struct trip_t {
    char *route_id;
    char *trip_id;          // trip_id du trajet
    double weight;          // Poids/durée du trajet
    TransportType type;     // Metro, bus, train
    unsigned int *horaires; // Liste des horaires de départ [9:00, 9:30, ...] en secondes depuis minuit (ex: 36000 pour 10:00)
    stop_t *dest;           // pointeur vers la station suivante
};

struct stop_t {
    char *stop_id;      // Stop_ID de la station
    char *stop_name;    // Nom de la station
    float stop_lat;     // latitude
    float stop_lon;     // longitude
    size_t nTrips;      // Nombre de trajets
    trip_t *trips;      // Liste de trajets pour cette station
};

struct graph_t {
    size_t nStops;      // Nombre de stations (noeuds)
    stop_t *stops;      // Tableau des noeuds (stations)
};

char *mmap_gtfs(gtfs_file_t *gtfs)
{
    info("Mapping '%s' into memory... ", gtfs->filepath);

    // Open the file in read only
    int fd = open(gtfs->filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Mapping into memory: read, shared to other processes
    char *mmapped = mmap(NULL, gtfs->filesize, PROT_READ, MAP_SHARED, fd, 0);
    if (mmapped == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    puts(GREEN "OK" RESET);
    return mmapped;
}

void munmap_gtfs(gtfs_file_t *gtfs)
{
    info("Unmapping '%s'... ", gtfs->filepath);

    // Open the file in read-only
    int fd = open(gtfs->filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (munmap(gtfs->data, gtfs->filesize) == -1) {
        perror("Error unmapping file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    puts(GREEN "OK" RESET);
}

off_t get_size(gtfs_file_t *gtfs)
{
    // Open the file in read only
    int fd = open(gtfs->filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Get the size of the file
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        exit(EXIT_FAILURE);
    }

    return file_size;
}


void add_station(graph_t *graph, const char *stop_id) {
    // Chercher si ce noeud existe déjà dans le graphe
    // Si non, ajouter un nouveau noeud avec le stop_id donné
}

// Créer une arête entre node_from et stop_id_to avec une certaine durée/poids
void add_trips(stop_t *stop, trip_t *trips) {
    stop->trips = (trip_t *)malloc(stop->nTrips * sizeof(trip_t));
    for (size_t i = 0; i < stop->nTrips; i++) {
        stop->trips[i] = trips[i];
    }

    info("Added %ld trips to %s\n", stop->nTrips, stop->stop_name);
}


/* pseudo-code
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
*/
