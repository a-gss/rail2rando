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
typedef struct {
    const char *filepath;
    char *data;
    size_t size;
} gtfs_file_t;


typedef struct {
    char *trip_id;
    char *service_id;
    char *route_id;
    char **stop_ids;         // stop_id list
    unsigned int stop_count; // Number of stops on the route
} trip_t;

typedef struct {
    char *name;
    char *stop_id;
    trip_t *trip_list;
    trip_t best_trip;
} trainstation_t;

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
    char *mmapped = mmap(NULL, gtfs->size, PROT_READ, MAP_SHARED, fd, 0);
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

    if (munmap(gtfs->data, gtfs->size) == -1) {
        perror("Error unmapping file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    puts(GREEN "OK" RESET);
}

size_t get_size(gtfs_file_t *gtfs)
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
