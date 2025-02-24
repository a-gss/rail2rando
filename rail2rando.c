/*/// TODO
    - rajouter le prix du trajet ça peut être sympa https://ressources.data.sncf.com/explore/dataset/tarifs-ouigo/table/
    - rajouter un warning "vous etes en train de calculer dans le passé"
/*///
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <fcntl.h>    // open()
#include <sys/mman.h> // mmap()
// #include <sys/stat.h>
#include <unistd.h>   // close(), lseek()
#include <regex.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define info(...) do { printf("[INFO] " __VA_ARGS__); fflush(stdout); } while (0)
#define error(...) logger(RED, "[ERROR] ", __VA_ARGS__)
#define warning(...) logger(MAGENTA, "[WARNING] ", __VA_ARGS__)

static inline void logger(const char *color, const char *level, const char *format, ...) {
    va_list args;
    printf("%s%s", color, level); // Print the color and level
    va_start(args, format); // Start processing the variadic arguments
    vprintf(format, args);  // Print the actual log message using the format string and arguments
    va_end(args);
    puts(RESET); // Reset color and add newline
    fflush(stdout);
}

#include "gtfs.c"
#include "regex.c"
#include "graph.c"

void help(char **argv) {
    printf("Usage : %s [options...] <origine>\n\n"

            "Arguments :\n"
            " <origine>                 Nom de la gare de départ (sensible à la casse)\n\n"

            "Options :\n"
            " -b, --bus                 Ajoute les bus dans le calcul d'itinéraire (le calcul sera plus long mais plus précis)\n"
            " -d, --date <yyyy-mm-dd>   Précise la date de départ (défaut: aujourd'hui)\n"
            "     --dest                Nom de la gare d'arrivée (insensible à la casse)\n"
            " -h, --heure <hh:mm>       Précise l'heure de départ (défaut: 00:00)\n"
            "     --help                Affiche ce texte\n"
            " -l, --link                Affiche le lien LIO du trajet\n\n"

            "Exemples :\n"
            " %s Matabiau\n"
            " %s -d 2025-09-25 --dest cahors saint-gaudens\n"
            " %s -h 09:45 -b --dest cahors saint-agne\n"
            " %s --bus -v --date 2025-04-16 --heure 09:45 --dest Matabiau --link Figeac\n",
            argv[0],
            argv[0],
            argv[0],
            argv[0],
            argv[0]
    );
}

void download_gtfs() {
    // TODO: only download if:
    //  - files do not exist
    //  - files are older than the valid data (updated tout les 5 mois un truc comme ça)
    info("Downloading GTFS data...\n");

    if (system("curl --parallel -L --create-dirs "
        "-s -o GTFS/TER/gtfs_ter.zip 'https://eu.ftp.opendatasoft.com/sncf/gtfs/export-ter-gtfs-last.zip' "
        "-s -o GTFS/INTERCITES/gtfs_intercites.zip 'https://eu.ftp.opendatasoft.com/sncf/plandata/export-intercites-gtfs-last.zip' "
        "-s -o GTFS/LIO/gtfs_lio.zip 'https://app.mecatran.com/utw/ws/gtfsfeed/static/lio?apiKey=2b160d626f783808095373766f18714901325e45&type=gtfs_lio' "
        "-w '[INFO] %{filename_effective} (%{size_download} bytes) DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total} \x1b[32m OK \x1b[0m \n' "
    ) == -1) {
        perror("Couldn't download GTFS data with CURL.");
    };

    // bon les TGV aussi mais si tu prends un putain de TGV pour aller en rando je sais pas cest quoi ton probleme

    if (system("unzip -oq -d GTFS/TER/ GTFS/TER/gtfs_ter.zip") == -1) { perror("Error unzipping GTFS");}
    if (system("unzip -oq -d GTFS/INTERCITES/ GTFS/INTERCITES/gtfs_intercites.zip") == -1) { perror("Error unzipping GTFS");}
    if (system("unzip -oq -d GTFS/LIO/ GTFS/LIO/gtfs_lio.zip") == -1) { perror("Error unzipping GTFS");}
}

int main(int argc, char **argv)
{
    // Initialize values
    bool flag_bus = false;
    bool flag_dest = false;
    bool flag_link_lio = false;
    // Get current time
    char date[11];            // yyyy-mm-dd\0
    char heure[6] = "00:00";  // hh:mm\0
    char dest[16] = "";
    // Init the date to today
    strftime(date, sizeof(date), "%F", localtime(&(time_t){time(NULL)}));
    stop_t orig;

    if (argc == 1) {
        error("Vous devez spécifier une gare de départ <origine>.");
        help(argv);
        return EXIT_FAILURE;
    }

    // Get the argument
    orig.stop_name = (char *)malloc(strlen(argv[argc - 1]) + 1);
    strcpy(orig.stop_name, argv[argc - 1]);

    // Get the options
    // TODO: refaire avec un switch ? https://stackoverflow.com/a/17509552 ou bien avec getopt()
    for (int i=1; i < argc - 1; i++) {
        if ((strcmp(argv[i], "--bus") == 0)  || (strcmp(argv[i], "-b") == 0)) {
            flag_bus = true;

        } else if ((strcmp(argv[i], "--date") == 0) || (strcmp(argv[i], "-d") == 0)) {
            strncpy(date, argv[++i], 10);

        } else if (strcmp(argv[i], "--dest") == 0) {
            flag_dest = true;
            strcpy(dest, argv[++i]);

        } else if ((strcmp(argv[i], "--heure") == 0) || (strcmp(argv[i], "-h") == 0)) {
            strncpy(heure, argv[++i], 5);

        } else if (strcmp(argv[i], "--help") == 0) {
            help(argv);
            return EXIT_SUCCESS;

        } else if ((strcmp(argv[i], "--link") == 0) || (strcmp(argv[i], "-l") == 0)) {
            flag_link_lio = true;

        } else {
            error("Option invalide '%s'.", argv[i]);
            help(argv);
            return EXIT_FAILURE;
        }
    }

    info("Départ de " YELLOW "%s" RESET " à %s le %s\n", orig.stop_name, heure, date);
    ///info("%s->%s [10:52]->[14:26] (3h34min)\n", orig, dest);

    // debug
    //printf("argc = %d\n", argc);
    //for (int i=0; i < argc; i++) printf("  argv[%d] = %s\n", i, argv[i]);
    //printf("date: %s\n", date);
    //printf("heure: %s\n", heure);
    //return 0;

    //download_gtfs();

    /*///   1. Find the ID of the stations
               Look up Toulouse Matabiau and Figeac in stops.txt to find their stop_id.
            2. Find all trains that stop in Toulouse Matabiau and depart AFTER the time we want.
               Check stop_times.txt for all trains stopping at Toulouse Matabiau and note their trip_id.
            3. See which of those trains also go to Figeac in stop_times.txt:
               Does the same trip_id that stops at Toulouse Matabiau also stop at Figeac later?
               If yes, this train is a valid option.
            4. Find the train with earliest departure time from Toulouse Matabiau.
            5. Check if the train runs that day in calendar.txt and calendar_dates.txt
               Each train has a service_id (from trips.txt).
    /*///   6. Print the results


    // 1. Find the ID of the stations
    gtfs_file_t gtfs_lio[GTFS_FILE_NUMBER];
    gtfs_lio[stops].filepath = gtfs_filepath[stops];
    gtfs_lio[stops].filesize = get_size(&gtfs_lio[stops]);
    gtfs_lio[stops].data = mmap_gtfs(&gtfs_lio[stops]);

    char stop_id_pattern[64];
    char **stop_id = NULL;
    size_t nStop_id = 0;
    snprintf(stop_id_pattern, sizeof(stop_id_pattern), "^([^,]*).*%s.*Gare|^([^,]*).*Gare.*%s", orig.stop_name, orig.stop_name);

    info("Searching for '%s' stations in '%s'... ", orig.stop_name, gtfs_lio[stops].filepath);
    nStop_id = regex_find(stop_id_pattern, &gtfs_lio[stops], 1, &stop_id, 0);
    if (nStop_id > 1) {
        info("%ld stop_id found:\n", nStop_id);
        for (size_t i = 0; i < nStop_id; i++) {
            info("  - %s\n", stop_id[i]);
        }
        info("The parent station is likely the first stop_id\n");

    } else {
        error("No result found for '%s'.", stop_id_pattern);
        exit(EXIT_FAILURE);
    }

    // Store the 1st stop_id
    orig.stop_id = (char *)malloc(strlen(stop_id[0]) + 1);
    if (orig.stop_id == NULL) {
        error("Failed to allocate memory for orig.stop_id");
        exit(EXIT_FAILURE);
    }
    strcpy(orig.stop_id, stop_id[0]);

    // Free stop_id
    for (size_t i = 0; i < nStop_id; i++) { free(stop_id[i]); }
    free(stop_id);

    info("'%s' stop_id: %s\n", orig.stop_name, orig.stop_id);

    // Graph shit
    graph_t graph;
    graph.nStops = count_lines(&gtfs_lio[stops]);
    info("%ld stops on the network\n", graph.nStops);
    graph.stops = (stop_t *)malloc(graph.nStops * sizeof(stop_t));

    munmap_gtfs(&gtfs_lio[stops]);

    info("Starting trip analysis...\n");
    // mmap routes.txt
    gtfs_lio[routes].filepath = gtfs_filepath[routes];
    gtfs_lio[routes].filesize = get_size(&gtfs_lio[routes]);
    gtfs_lio[routes].data = mmap_gtfs(&gtfs_lio[routes]);
    // mmap trips.txt
    gtfs_lio[trips].filepath = gtfs_filepath[trips];
    gtfs_lio[trips].filesize = get_size(&gtfs_lio[trips]);
    gtfs_lio[trips].data = mmap_gtfs(&gtfs_lio[trips]);

    info("Searching how much routes are in '%s'... ", gtfs_lio[routes].filepath);
    char **route_id_list = NULL;
    size_t nRoutes = regex_find("^[^,]*", &gtfs_lio[routes], 0, &route_id_list, 0);

    char trip_id_pattern[64];
    for (size_t i = 0; i < nRoutes; i++) {
    //for (size_t i = 0; i < 6; i++) {
        //info("Getting trips for route [%ld]%s... ", i, route_id_list[i]);
        snprintf(trip_id_pattern, sizeof(trip_id_pattern), "^%s,[^,]*,([^,]*)", route_id_list[i]);
        char **trip_id_list = NULL;
        graph.stops[i].nTrips = regex_find(trip_id_pattern, &gtfs_lio[trips], 1, &trip_id_list, 32);

        // Put the trip_ids found in the graph.stop
        graph.stops[i].trips = (trip_t *)malloc(graph.stops[i].nTrips * sizeof(trip_t));
        for (size_t j = 0; j < graph.stops[i].nTrips; j++) {
            graph.stops[i].trips[j].trip_id = (char *)malloc(strlen(trip_id_list[j]));
            strcpy(graph.stops[i].trips[j].trip_id, trip_id_list[j]);
            //info(" [%ld-%03ld] %s\n", i, j+1, graph.stops[i].trips[j].trip_id);
        }
    }

    munmap_gtfs(&gtfs_lio[stops]);
    munmap_gtfs(&gtfs_lio[trips]);

    free(orig.stop_name);
    free(orig.stop_id);

    puts("--------------------------------------------------------------------------------");
    return EXIT_SUCCESS;
}
