#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>   // stat, S_ISDIR, mkdir
#include <limits.h>     // PATH_MAX - for maximum path length
#include <unistd.h>     // getcwd

int main(int argc, char **argv) {
    (void)argc; (void)argv; // Silence unused parameter warnings cleanly

    // 1. Capture PROJECT_DIR state
    char project_dir[PATH_MAX]; // Buffer to hold the current working directory
    if (getcwd(project_dir, sizeof(project_dir)) == NULL) {
        perror("getcwd() error");
        return 1;
    }

    // 2. Read target environment context 
    char *backup_target = getenv("BACKUP_DIR"); // j
    if (backup_target == NULL) {
        fprintf(stderr, "BACKUP_DIR environment variable is not set.\n");
        return 1;
    }

    // 3. File metadata verification
    struct stat path_stat;
    if (stat(backup_target, &path_stat) != 0) {
        perror("Error accessing BACKUP_DIR target");
        return 1;
    }
    int is_directory = S_ISDIR(path_stat.st_mode);

    // 4. Enforce archive container existence
    char archive_dir[PATH_MAX];
    snprintf(archive_dir, sizeof(archive_dir), "%s/archive", project_dir);

    struct stat archive_stat;
    if (stat(archive_dir, &archive_stat) != 0) {
        if (mkdir(archive_dir, 0755) != 0) {
            perror("Failed to create archive directory");
            return 1;
        }
    }

    // 5. Generate deterministic timestamp tokens
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);

    // Isolate base directory/file title name 
    char target_copy[PATH_MAX];
    strncpy(target_copy, backup_target, sizeof(target_copy) - 1);
    target_copy[sizeof(target_copy) - 1] = '\0';
    char *base_name = strrchr(target_copy, '/');
    base_name = (base_name != NULL) ? base_name + 1 : target_copy;

    // Handle trailing slash edge case if user input ends with '/'
    if (strlen(base_name) == 0) {
        base_name = "backup_item";
    }

    char zip_filename[PATH_MAX];
    snprintf(zip_filename, sizeof(zip_filename), "%s_%s.zip", base_name, timestamp);

    char zip_full_path[PATH_MAX];
    snprintf(zip_full_path, sizeof(zip_full_path), "%s/%s", archive_dir, zip_filename);

    // 6. Build and execute target compression pipeline
    char command[PATH_MAX * 2];
    if (is_directory) {
        // -r: recursive archiving for directories
        snprintf(command, sizeof(command), "zip -r \"%s\" \"%s\"", zip_full_path, backup_target);
    } else {
        // -j: junk paths so single files don't carry annoying directory structures inside the zip
        snprintf(command, sizeof(command), "zip -j \"%s\" \"%s\"", zip_full_path, backup_target);
    }

    int ret = system(command);
    if (ret != 0) {
        fprintf(stderr, "zip command failed with exit code %d\n", ret);
        return 1;
    }

    // 7. Output operational performance diagnostics
    printf("Backup complete.\n");
    printf("Source: %s (%s)\n", backup_target, is_directory ? "directory" : "file");
    printf("Archive: %s\n", zip_full_path);

    return 0;
}