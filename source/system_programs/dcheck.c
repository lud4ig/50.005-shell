#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    // ps -efj   :yeah  full-format listing including session/process-group info
    // grep dspawn       : keep lines mentioning dspawn
    // grep -v grep      : drop the grep command itself from matching its own argument
    // grep -Ev 'tty|pts' : drop any process still attached to a terminal
    const char *cmd = "ps -efj | grep dspawn | grep -v grep | grep -Ev 'tty|pts'";

    FILE *pipe = popen(cmd, "r"); // Open a pipe to execute the command and read its output
    if (pipe == NULL) {
        perror("popen failed");
        return 1;
    }

    char line[256];
    int count = 0;

    // Read each line of output from the command and count the number of lines, which corresponds to the number of live daemons
    while (fgets(line, sizeof(line), pipe) != NULL) { 
        count++;
        printf("%s", line); // Print each line of output for debugging purposes
    }

    int status = pclose(pipe); // Close the pipe and get the exit status of the command
    if (status == -1) {
        perror("pclose failed");
        return 1;
    }

    printf("Live daemons: %d\n", count);

    return 0;
}