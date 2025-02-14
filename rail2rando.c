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
#define warning(...) do { printf(YELLOW "[WARNING] " RESET __VA_ARGS__); fflush(stdout); } while (0)
static inline void error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, RED "[ERROR] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, RESET "\n");
    va_end(args);
    fflush(stdout);
}

#include "gtfs.c"
#include "regex.c"

void help(char **argv) {
    printf("Usage : %s [options...] <origine>\n\n"

            "Arguments :\n"
            " <origine>                 Nom de la gare de départ (insensible à la casse)\n\n"

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

int main(int argc, char **argv)
{
    // Initialize values
    bool flag_bus = false;
    bool flag_link_lio = false;
    // Get current time
    char date[11];            // yyyy-mm-dd\0
    char heure[6] = "00:00";  // hh:mm\0
    char orig[16];
    char dest[16] = "";
    // Init the date to today
    strftime(date, sizeof(date), "%F", localtime(&(time_t){time(NULL)}));

    if (argc == 1) {
        error("Vous devez spécifier une gare de départ <origine>.");
        help(argv);
        return EXIT_FAILURE;
    }

    // Get the argument
    strcpy(orig, argv[argc - 1]);

    // Get the options
    // TODO: refaire avec un switch ? https://stackoverflow.com/a/17509552 ou bien avec getopt()
    for (int i=1; i < argc - 1; i++) {
        if ((strcmp(argv[i], "--bus") == 0)  || (strcmp(argv[i], "-b") == 0)) {
            flag_bus = true;

        } else if ((strcmp(argv[i], "--date") == 0) || (strcmp(argv[i], "-d") == 0)) {
            strncpy(date, argv[++i], 10);

        } else if (strcmp(argv[i], "--dest") == 0) {
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

    info("Départ de " YELLOW "%s" RESET " à %s le %s\n", orig, heure, date);
    ///info("%s->%s [10:52]->[14:26] (3h34min)\n", orig, dest);

    // debug
    //printf("argc = %d\n", argc);
    //for (int i=0; i < argc; i++) printf("  argv[%d] = %s\n", i, argv[i]);
    //printf("date: %s\n", date);
    //printf("heure: %s\n", heure);
    //return 0;

    // TODO: only download if:
    //  - files do not exist
    //  - files are older than the valid data (updated tout les 5 mois un truc comme ça)
    info("Downloading GTFS data... ");
    //system("curl --parallel -L --create-dirs "
    //   "-s -o GTFS/calendar.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/9bd5ef79fa139ccbce0511108584394b/download/' "
    //   "-s -o GTFS/calendar_dates.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/d39dc8f4edb7fa0cb7ed377a4b0f11f6/download/' "
    //   "-s -o GTFS/routes.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/92c45d9df99624d7e05e9ade35ba0ce8/download/' "
    //   "-s -o GTFS/stop_times.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/3cc9124c230b72e07df09e27c59eba88/download/' "
    //   "-s -o GTFS/stops.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7068c8d492df76c5125fac081b5e09e9/download/' "
    //   "-s -o GTFS/trips.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7831854a320cbf4ea5b6b327cd4581af/download/' "
    //);
    puts(GREEN "OK" RESET);

    // mmap() GTFS files for fast access
    //  Bon je crois bien que le GTFS LIO-Occitanie c'est que les arrets de bus
    //  il faut donc rajouter les TER et les intercités
    //  https://ressources.data.sncf.com/explore/dataset/sncf-intercites-gtfs/information/
    //  https://ressources.data.sncf.com/explore/dataset/sncf-ter-gtfs/information/
    //  et bon les TGV aussi mais si tu prends un putain de TGV pour aller en rando je sais
    //  pas cest quoi ton probleme
    char *gtfs[GTFS_FILE_NUMBER];
    //gtfs[calendar] = mmap_gtfs("GTFS/calendar.txt");
    //gtfs[calendar_dates] = mmap_gtfs("GTFS/calendar_dates.txt");
    //gtfs[routes] = mmap_gtfs("GTFS/routes.txt");
    //gtfs[stop_times] = mmap_gtfs("GTFS/stop_times.txt");
    //gtfs[trips] = mmap_gtfs("GTFS/trips.txt");


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
    gtfs[stops] = mmap_gtfs(gtfs_filepath[stops]);

    char stop_id_pattern[64];
    char stop_id[1][64];
    snprintf(stop_id_pattern, sizeof(stop_id_pattern), "^([^,]*).*%s", orig);

    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[stops]);
    if (regex_find(stop_id_pattern, gtfs[stops], 1, stop_id) == true) {
        info("%s STOP_ID: %s\n", orig, stop_id[0]);

    } else {
        error("Aucun résultat trouvé pour '%s'.", stop_id_pattern);
        exit(EXIT_FAILURE);
    }

    munmap_gtfs(gtfs_filepath[stops], gtfs[stops]);



/*
    info("Searching for '%s' ID in '/GTFS/stops.txt'... ", orig);
    char stop_id_pattern[128];
    char stop_id[9] = "";
    snprintf(stop_id_pattern, sizeof(stop_id_pattern), "^([^,]*).*%s", orig);

    regex_t regex;
    if (regcomp(&regex, "31S16535", REG_EXTENDED) != 0) {
        error("Impossible de compiler le regex.");
        return EXIT_FAILURE;
    }

    regmatch_t matches[2]; // Capture group for stop_id
    char *save_ptr = NULL;
    char *line = strtok_r(gtfs_data[stops], "\n", &save_ptr);
    while (line) {
        if (regexec(&regex, line, 2, matches, 0) == 0) {
            int len = matches[1].rm_eo - matches[1].rm_so;
            strncpy(stop_id, line + matches[1].rm_so, len);
            stop_id[len] = '\0'; // Ensure null termination
            printf("FOUND (%s)\n", stop_id);
            break; // Stop after the first match
        }
        line = strtok_r(NULL, "\n", &save_ptr);
    }
    puts(RED "NOT FOUND" RESET);



    if (strcmp(stop_id, "31S16535") != 0)
    //error("\npas le bon ID llolloll pont matabiau i guess\n");




    regfree(&regex);
    return 0;


    int i;
    //printf("%s", gtfs_data[stops]);
    while(1) {
        if (gtfs_data[stops][i] == '\0') {
            printf("\\0\n");
            break;
        } else if (gtfs_data[stops][i] == '\n') {
            printf("\\n");
        }
        putchar(gtfs_data[stops][i]); // Access the data like an array
        i++;
    }
    info("%s ID: %d\n", orig, 3);

*/


    // ----------------------------------------------------------
    // Unmap GTFS files when done

    //munmap_gtfs(gtfs_filepath[calendar], gtfs[calendar]);
    //munmap_gtfs(gtfs_filepath[calendar_dates], gtfs[calendar_dates]);
    //munmap_gtfs(gtfs_filepath[routes], gtfs[routes]);
    //munmap_gtfs(gtfs_filepath[stop_times], gtfs[stop_times]);
    //munmap_gtfs(gtfs_filepath[stops], gtfs[stops]);
    //munmap_gtfs(gtfs_filepath[trips], gtfs[trips]);
    printf("-----------------------------------------------------\n");
    return EXIT_SUCCESS;
}
