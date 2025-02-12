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
    info("Mapping '%s' into memory... ", filepath);

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
    puts("OK");
    return mmapped;
}

void munmap_gtfs(char *filepath)
{
    info("Unmapping '%s'... ", filepath);

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

    puts("OK");
}
