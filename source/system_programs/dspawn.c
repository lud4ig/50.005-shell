#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>   // umask
#include <fcntl.h>      // open
#include <string.h>
#include <limits.h>     // PATH_MAX

static char log_path[PATH_MAX];

// Step 9: the actual work the daemon performs
static int daemon_work(void) {
    int num = 0;
    FILE *fptr;
    char buffer[1024];
    char *cwd;

    // write PID of daemon at the start
    fptr = fopen(log_path, "a"); // Open the log file in append mode to write the PID of the daemon process
    if (fptr == NULL) {
        return EXIT_FAILURE;
    }
    // Log the PID and PPID of the daemon process, along with the file descriptor of the log file
    fprintf(fptr, "Daemon process running with PID: %d, PPID: %d, opening logfile with FD %d\n",
            getpid(), getppid(), fileno(fptr)); 

    // Get the current working directory and log it to the file
    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd == NULL) {
        fclose(fptr);
        return EXIT_FAILURE;
    }
    fprintf(fptr, "Current working directory: %s\n", cwd);
    fclose(fptr);

    while (1) {
        fptr = fopen(log_path, "a");
        if (fptr == NULL) {
            return EXIT_FAILURE;
        }
        fprintf(fptr, "PID %d Daemon writing line %d to the file.\n", getpid(), num);
        num++;
        fclose(fptr);

        sleep(10);

        if (num == 10) // toy daemon terminates after 10 counts, for lab safety
            break;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    // Capture the log path BEFORE any fork/chdir, while cwd is still PROJECT_DIR
    if (getcwd(log_path, sizeof(log_path)) == NULL) {
        perror("getcwd() error, exiting now.");
        return 1;
    }
    strcat(log_path, "/dspawn.log");

    // first fork to create an intermediate process (step 1)
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid1 > 0) {
        // exit dspawn, shell regains control, intermediate process continues (step 21)
        exit(0);
    }

    // inside intermediate process now

    // setsid - become session leader, lose controlling TTY (step 3)
    if (setsid() < 0) {
        perror("setsid failed");
        exit(1);
    }

    // Step 4: ignore SIGCHLD and SIGHUP 
    signal(SIGCHLD, SIG_IGN); // Ignore SIGCHLD to prevent zombie processes, reaped when daemon terminates
    signal(SIGHUP, SIG_IGN); // prevent termination when controlling terminal is closed

    // Step 5: second fork to create daemon process, intermediate process exits 
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("second fork failed");
        exit(1);
    }

    if (pid2 > 0) {
        // intermediate process terminates, ensuring the daemon is not a session leader
        // ensures that daemon PID != session ID, preventing it from accidentally acquiring a controlling terminal
        exit(0);
    }

    // In daemon process now, which is not a session leader and has no controlling terminal

    // Step 6: set file permissions
    // newly created files or directories will have all permissions set
    umask(0); // All files created as permission 0777 (world-RW and executable) -> translates to 111 111 111 in binary, which is rwxrwxrwx in symbolic notation

    // Step 7: change working directory to root
    // Prevent directory from being unmounted
    if (chdir("/") < 0) {
        // can't log yet (fds not redirected), just exit
        exit(1);
    }

    // Step 8: close all open fds, then attach 0,1,2 to /dev/null 
    // Ensures that daemon doesn't accidentally read from or write to the terminal
    // Also ensures that daemon doesn't hold any files open that it shouldn't
    int x;
    // sysconf(_SC_OPEN_MAX) returns the maximum number of file descriptors 
    // for loop iterates through all possible file descriptors and closes them
    // This is a common practice in daemonization to ensure that the daemon doesn't unintentionally keep any file descriptors open 
    // that it inherited from the parent process, which could lead to resource leaks or unintended interactions with files or terminals
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
    // Redirect standard input, output, and error to /dev/null
    int fd0 = open("/dev/null", O_RDWR); // becomes fd 0, silence any input from terminal
    int fd1 = dup(0);                    // becomes fd 1, silence any output to terminal
    int fd2 = dup(0);                    // becomes fd 2, silence any error output to terminal
    (void)fd0; (void)fd1; (void)fd2;      // silence unused-variable warnings

    // ---- Step 9: execute daemon_work() ----
    return daemon_work();
}