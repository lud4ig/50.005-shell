#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>  // For uname (OS and Kernel info)
#ifdef __linux__
# include <sys/sysinfo.h>  // For sysinfo (Memory info) - available on Linux
#endif
#include <string.h>       // For strncmp
#include <unistd.h>       // For standard system configurations

int main(int argc, char **argv) {
    // Get the current user from the environment variable
    char *user = getenv("USER");
    if (user == NULL) {
        user = "Unknown";
    }
    printf("User: %s\n", user);

    // Print the current working directory
    // creating a struct to hold system information
    // struct is a data structure that can hold multiple related variables of different types. 
    struct utsname sys_info; 
    if (uname(&sys_info) == 0) { // Get system information and check for success
        printf("System: %s\n", sys_info.sysname);
        printf("Node Name: %s\n", sys_info.nodename);
        printf("Release: %s\n", sys_info.release);
        printf("Version: %s\n", sys_info.version);
        printf("Machine: %s\n", sys_info.machine);
        printf("Architecture: %s\n", sys_info.machine); // Print the machine architecture
        long num_cores = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of CPU cores
        printf("CPU Cores: %ld\n", num_cores);
     } else {
        perror("uname failed");
     }
     // Read and print memory information (Linux-specific)
     #ifdef __linux__
        struct sysinfo mem_info;
        if (sysinfo(&mem_info) == 0) {
            // Safely scale up by the memory unit multiplier to get the total and free memory in bytes
            // Cast to unsigned long long to prevent overflow when multiplying by mem_unit
            // Unsigned long long is a data type that can hold larger positive integer values
            unsigned long long total_bytes = (unsigned long long)mem_info.totalram * mem_info.mem_unit;
            unsigned long long free_bytes = (unsigned long long)mem_info.freeram * mem_info.mem_unit;
            // Convert bytes to MB for display
            printf("Total RAM: %llu MB\n", total_bytes / (1024 * 1024)); 
            printf("Free RAM: %llu MB\n", free_bytes / (1024 * 1024)); 
        } else {
            perror("sysinfo failed");
        }
    #else
        printf("Memory information is not available on this platform.\n");
    #endif
// Read and print CPU information
FILE *cpu_file = fopen("/proc/cpuinfo", "r"); // Reads CPU information
char line[256];

if (cpu_file != NULL) {
    while (fgets(line, sizeof(line), cpu_file)) {
        // Look for the "model name" string, line means text read from file, strncmp compares the first 10 characters of line with "model name"
        if (strncmp(line, "model name", 10) == 0) {
            // Extract and print the CPU model name here, then break
            printf("CPU: %s", line + 13); // Print the CPU model name, line + 13 skips the "model name : " part
            break;
        }
    }
    fclose(cpu_file);
}else{
    perror("Failed to open /proc/cpuinfo");
}
    return 0;
}