#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <regex.h>

#define info(...)     do { printf("[INFO] "    __VA_ARGS__); fflush(stdout); } while (0)
#define warning(...)  do { printf("[WARNING] " __VA_ARGS__); fflush(stdout); } while (0)
#define error(...)    do { printf("[ERROR] "   __VA_ARGS__); fflush(stdout); } while (0)

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
            " %s --bus --date 2025-04-16 --heure 09:45 --dest Matabiau --link Figeac\n",
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
    char date[11];             // yyyy-mm-dd\0
    char heure[6] = "00:00";  // hh:mm\0
    char orig[16];
    char dest[16] = "";
    // Init the date to today
    strftime(date, sizeof(date), "%F", localtime(&(time_t){time(NULL)}));


    if (argc == 1) {
        error("Vous devez spécifier une gare de départ <origine>.\n");
        help(argv);
        return EXIT_FAILURE;
    }

    // Get the argument
    strcpy(orig, argv[argc - 1]);

    // Get the options
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
            error("Invalid option.\n");
            help(argv);
            return EXIT_FAILURE;
        }
    }

    info("Depart de %s à %s le %s\n", orig, heure, date);
    ///info("%s->%s [10:52]->[14:26] (3h34min)\n", orig, dest);

    // debug
    printf("argc = %d\n", argc);
    for (int i=0; i < argc; i++) printf("  argv[%d] = %s\n", i, argv[i]);
    printf("date: %s\n", date);
    printf("heure: %s\n", heure);
    return 0;



    enum gtfs_index {
        calendar,
        calendar_dates,
        routes,
        stop_times,
        stops,
        trips,
        GTFS_FILE_NUMBER
    };

    const char* gtfs_filenames[] = {
        "GTFS/calendar.txt",
        "GTFS/calendar_dates.txt",
        "GTFS/routes.txt",
        "GTFS/stop_times.txt",
        "GTFS/stops.txt",
        "GTFS/trips.txt",
    };

    // TODO: only download if:
    //  - files do not exist
    //  - files are older than the valid data (updated tout les 5 mois un truc comme ça)
    info("Downloading GTFS data... ");
    system("curl --parallel -L --create-dirs "
       "-s -o GTFS/calendar.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/9bd5ef79fa139ccbce0511108584394b/download/' "
       "-s -o GTFS/calendar_dates.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/d39dc8f4edb7fa0cb7ed377a4b0f11f6/download/' "
       "-s -o GTFS/routes.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/92c45d9df99624d7e05e9ade35ba0ce8/download/' "
       "-s -o GTFS/stop_times.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/3cc9124c230b72e07df09e27c59eba88/download/' "
       "-s -o GTFS/stops.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7068c8d492df76c5125fac081b5e09e9/download/' "
       "-s -o GTFS/trips.txt 'https://data.laregion.fr/explore/dataset/reseau-lio/files/7831854a320cbf4ea5b6b327cd4581af/download/' "
    );
    puts("OK");

    char *gtfs_data[GTFS_FILE_NUMBER];
    for (int i=0; i<GTFS_FILE_NUMBER; ++i) {
        info("Mapping '%s' into memory... ", gtfs_filenames[i]);

        // Open the file in read-only
        int fd = open(gtfs_filenames[i], O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        // Get the size of the file
        off_t file_size = lseek(fd, 0, SEEK_END);
        if (file_size == -1) {
            perror("Error getting file size");
            return EXIT_FAILURE;
        }

        // Mapping into memory: read-only, shared to other processes
        gtfs_data[i] = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
        if (gtfs_data == MAP_FAILED) {
            perror("Error mapping file");
            close(fd);
            return EXIT_FAILURE;
        }

        close(fd);
        puts("OK");
    }

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
    printf("calendar dates:\n");
    int i;
    while(gtfs_data[calendar_dates][i] != '\0') {
        putchar(gtfs_data[calendar_dates][i++]); // Access the data like an array
    }





    // -----------------------------------------------------
    // Unmap the files when done
    for (int i=0; i<GTFS_FILE_NUMBER; ++i) {
        info("Unmapping '%s'... ", gtfs_filenames[i]);

        // Open the file in read-only
        int fd = open(gtfs_filenames[i], O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        // Get the size of the file
        off_t file_size = lseek(fd, 0, SEEK_END);
        if (file_size == -1) {
            perror("Error getting file size");
            return EXIT_FAILURE;
        }

        if (munmap(gtfs_data[i], file_size) == -1) {
            perror("Error unmapping file");
            close(fd);
            return EXIT_FAILURE;
        }

        puts("OK");
    }

    printf("-----------------------------------------------------\n");
    return EXIT_SUCCESS;
}
