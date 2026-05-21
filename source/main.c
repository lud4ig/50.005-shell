#include "shell.h"

// The main function where the shell's execution begins
int main(void)
{
    // Define an array to hold the command and its arguments
    char *cmd[MAX_ARGS];
    int child_status;
    pid_t pid;

    type_prompt();     // Display the prompt
    read_command(cmd); // Read a command from the user

    // If the command is "exit", break out of the loop to terminate the shell
    if (strcmp(cmd[0], "exit") == 0)
        // break;
        return 0;

    // Formulate the full path of the command to be executed
    char full_path[PATH_MAX];
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {

        snprintf(full_path, sizeof(full_path), "%s/bin/%s", cwd, cmd[0]);
    }
    else
    {
        printf("Failed to get current working directory.");
        exit(1);
    }

    execv(full_path, cmd);

    // If execv returns, command execution has failed
    printf("Command %s not found\n", cmd[0]);
    exit(0);
}