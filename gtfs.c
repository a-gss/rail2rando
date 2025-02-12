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

char *mmap_gtfs(char *filepath)
{
    info("Mapping " YELLOW "'%s'" RESET " into memory... ", filepath);

    // Open the file in read/write
    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Get the size of the file
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        exit(1);
    }

    // Mapping into memory: read/write, shared to other processes
    char *mmapped = mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmapped == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    close(fd);
    puts(GREEN "OK" RESET);
    return mmapped;
}

void munmap_gtfs(char *filepath)
{
    info("Unmapping " YELLOW "'%s'" RESET "... ", filepath);

    // Open the file in read-only
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Get the size of the file
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        exit(1);
    }

    if (munmap(filepath, file_size) == -1) {
        perror("Error unmapping file");
        close(fd);
        exit(1);
    }

    puts(GREEN "OK" RESET);
}

void regex_wrapper(const char *str, const char *pattern,
                   int n_capturing_group, char result[][64])
{
    regex_t regex;
    regmatch_t matches[n_capturing_group + 1];  // Index 0 = match complet

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        error("Erreur de compilation du regex\n");
        return;
    }

    info("Searching for " YELLOW "'%s'" RESET " in " YELLOW "'%s'" RESET "... ", str, pattern);
    if (regexec(&regex, str, n_capturing_group + 1, matches, 0) == 0) {
        if (n_capturing_group == 0) {
            // Si aucun groupe capturant, récupérer le match complet
            int debut = matches[0].rm_so;
            int fin = matches[0].rm_eo;
            int len = fin - debut;

            strncpy(result[0], str + debut, len);
            result[0][len] = '\0';
            printf(GREEN "MATCH: %s\n" RESET, result[0]);

        } else { // Boucle sur les groupes capturés
            printf(GREEN "MATCH: ");
            for (int i = 1; i <= n_capturing_group; i++) {
                if (matches[i].rm_so != -1) { // Vérifie si le groupe est capturé
                    int debut = matches[i].rm_so;
                    int fin = matches[i].rm_eo;
                    int len = fin - debut;

                    strncpy(result[i-1], str + debut, len);
                    result[i-1][len] = '\0';
                    printf("(%d)%s ", i, result[i-1]);
                }
            }
            printf("\n" RESET);
        }

    } else {
        puts(RED "NO MATCH" RESET);
    }

    regfree(&regex);
}
