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


    system("curl --parallel -L --create-dirs "
        "-s -o GTFS/TER/gtfs_ter.zip 'https://eu.ftp.opendatasoft.com/sncf/gtfs/export-ter-gtfs-last.zip' "
        "-s -o GTFS/INTERCITES/gtfs_intercites.zip 'https://eu.ftp.opendatasoft.com/sncf/plandata/export-intercites-gtfs-last.zip' "
        "-s -o GTFS/LIO/gtfs_lio.zip 'https://app.mecatran.com/utw/ws/gtfsfeed/static/lio?apiKey=2b160d626f783808095373766f18714901325e45&type=gtfs_lio' "
        "-w '[INFO] %{filename_effective} (%{size_download} bytes) DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total} \x1b[32m OK \x1b[0m \n' "
    );

    // bon les TGV aussi mais si tu prends un putain de TGV pour aller en rando je sais pas cest quoi ton probleme

    system("unzip -oq -d GTFS/TER/ GTFS/TER/gtfs_ter.zip");
    system("unzip -oq -d GTFS/INTERCITES/ GTFS/INTERCITES/gtfs_intercites.zip");
    system("unzip -oq -d GTFS/LIO/ GTFS/LIO/gtfs_lio.zip");

    /*
        system("curl --parallel -L --create-dirs "
        system("unzip -o export-ter-gtfs-last.zip -d GTFS_TER");
        system("unzip -o export-intercites-gtfs-last.zip -d GTFS_INTERCITES");
        system("unzip -o lio.zip -d GTFS_LIO");

        "--resolve data.laregion.fr:443:109.232.232.161 "
        "-H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0' "
        "-H 'X-Requested-With: XMLHttpRequest' "
        "-H 'Connection: keep-alive' "
        "-s -o GTFS/calendar.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/9bd5ef79fa139ccbce0511108584394b/download/' "
        "-s -o GTFS/calendar_dates.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/d39dc8f4edb7fa0cb7ed377a4b0f11f6/download/' "
        "-s -o GTFS/routes.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/92c45d9df99624d7e05e9ade35ba0ce8/download/' "
        "-s -o GTFS/stop_times.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/3cc9124c230b72e07df09e27c59eba88/download/' "
        "-s -o GTFS/stops.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7068c8d492df76c5125fac081b5e09e9/download/' "
        "-s -o GTFS/trips.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7831854a320cbf4ea5b6b327cd4581af/download/' "
        "-w '[INFO] %{filename_effective} (%{size_download} bytes) DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total} \x1b[32m OK \x1b[0m \n' "
        );
    */

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
    trainstation_t orig;

    if (argc == 1) {
        error("Vous devez spécifier une gare de départ <origine>.");
        help(argv);
        return EXIT_FAILURE;
    }

    // Get the argument
    orig.name = (char *)malloc(sizeof(argv[argc - 1]));
    strcpy(orig.name, argv[argc - 1]);

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

    info("Départ de " YELLOW "%s" RESET " à %s le %s\n", orig.name, heure, date);
    ///info("%s->%s [10:52]->[14:26] (3h34min)\n", orig, dest);

    // debug
    //printf("argc = %d\n", argc);
    //for (int i=0; i < argc; i++) printf("  argv[%d] = %s\n", i, argv[i]);
    //printf("date: %s\n", date);
    //printf("heure: %s\n", heure);
    //return 0;

//    download_gtfs();

    // mmap() GTFS files for fast access
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
    char **stop_id = NULL;
    unsigned int stop_id_count = 0;
    snprintf(stop_id_pattern, sizeof(stop_id_pattern), "^([^,]*).*%s.*Gare|^([^,]*).*Gare.*%s", orig.name, orig.name);
    //snprintf(stop_id_pattern, sizeof(stop_id_pattern), "^([^,]*).*%s", orig.name);

    info("Searching for '%s' trainstations in '%s'... ", orig.name, gtfs_filepath[stops]);
    stop_id_count = regex_find(stop_id_pattern, gtfs[stops], 1, &stop_id);
    if (stop_id_count > 1) {
        info("%d trainstations found:\n", stop_id_count);
        for (int i = 0; i < stop_id_count; i++) {
            info("  - %s STOP_ID: %s\n", orig.name, stop_id[i]);
        }
        info("The parent station is likely the first STOP_ID\n");

    } else {
        error("Aucun résultat trouvé pour '%s'.", stop_id_pattern);
        exit(EXIT_FAILURE);
    }

    // Store the 1st stop_id
    orig.stop_id = malloc(strlen((stop_id[0]) + 1) * sizeof(char));
    if (orig.stop_id == NULL) {
        error("Failed to allocate memory for orig.stop_id");
        exit(EXIT_FAILURE);
    }
    strcpy(orig.stop_id, stop_id[0]);

    // Free stop_id
    for (int i = 0; i < stop_id_count; i++) { free(stop_id[i]); }
    free(stop_id);

    info("%s STOP_ID: %s\n", orig.name, orig.stop_id);

    munmap_gtfs(gtfs_filepath[stops], gtfs[stops]);




// Juste pour benchmark le regex sur les fichiers
/*
    gtfs[calendar] = mmap_gtfs(gtfs_filepath[calendar]);
    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[calendar]);
    regex_find(stop_id_pattern, gtfs[calendar], 1, stop_id);
    munmap_gtfs(gtfs_filepath[calendar], gtfs[calendar]);

    gtfs[calendar_dates] = mmap_gtfs(gtfs_filepath[calendar_dates]);
    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[calendar_dates]);
    regex_find(stop_id_pattern, gtfs[calendar_dates], 1, stop_id);
    munmap_gtfs(gtfs_filepath[calendar_dates], gtfs[calendar_dates]);

    gtfs[routes] = mmap_gtfs(gtfs_filepath[routes]);
    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[routes]);
    regex_find(stop_id_pattern, gtfs[routes], 1, stop_id);
    munmap_gtfs(gtfs_filepath[routes], gtfs[routes]);

    gtfs[stop_times] = mmap_gtfs(gtfs_filepath[stop_times]);
    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[stop_times]);
    regex_find(stop_id_pattern, gtfs[stop_times], 1, stop_id);
    munmap_gtfs(gtfs_filepath[stop_times], gtfs[stop_times]);

    gtfs[trips] = mmap_gtfs(gtfs_filepath[trips]);
    info("Searching for '%s' in '%s'... ", stop_id_pattern, gtfs_filepath[trips]);
    regex_find(stop_id_pattern, gtfs[trips], 1, stop_id);
    munmap_gtfs(gtfs_filepath[trips], gtfs[trips]);
*/
    free(orig.name);

    printf("--------------------------------------------------------------------------------");
    return EXIT_SUCCESS;
}
