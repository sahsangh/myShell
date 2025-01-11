#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <glob.h>

#define BUFFER_SIZE 1024
#define SEARCH_PATHS 3

// Function declarations
void print_welcome();
void print_goodbye();
void execute_command(char *command, int interactive_mode);
void parse_and_execute(char *command, int interactive_mode);
void handle_redirection_and_execute(char **args, char *input_file, char *output_file, int interactive_mode);
void handle_pipeline(char **left_command, char **right_command, int interactive_mode, char *output_file);
int is_builtin_command(char *command);
void execute_builtin_command(char **args);
char *search_executable(char *command);
void expand_wildcards(char **args, int *arg_count);

int main(int argc, char *argv[])
{
    int interactive_mode = isatty(STDIN_FILENO);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    FILE *input_file = NULL;

    // Handle input source (interactive or batch mode)
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 2)
    {
        input_file = fopen(argv[1], "r");
        if (!input_file)
        {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        interactive_mode = 0;
    }

    if (interactive_mode)
    {
        print_welcome();
    }

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        if (interactive_mode)
        {
            printf("mysh> ");
            fflush(stdout);
        }

        // Read input
        if (input_file)
        {
            bytes_read = read(fileno(input_file), buffer, BUFFER_SIZE - 1);
            if (bytes_read < 0)
            {
                perror("Error reading input");
                continue;
            }
            else if (bytes_read == 0)
            {
                break; // End of file
            }
            buffer[bytes_read] = '\0';

            // Process the buffer line by line
            char *line_start = buffer;
            char *newline;
            while ((newline = strchr(line_start, '\n')) != NULL)
            {
                *newline = '\0';
                if (strcmp(line_start, "exit") == 0)
                {
                    return 0; // Exit cleanly
                }
                parse_and_execute(line_start, interactive_mode);
                line_start = newline + 1;
            }
            // If there's remaining text without a newline, process it
            if (*line_start != '\0')
            {
                if (strcmp(line_start, "exit") == 0)
                {
                    return 0; // Exit cleanly
                }
                parse_and_execute(line_start, interactive_mode);
            }
        }
        else
        {
            bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
            if (bytes_read < 0)
            {
                perror("Error reading input");
                continue;
            }
            else if (bytes_read == 0)
            {
                break; // End of input
            }

            // Remove trailing newline in interactive mode
            if (bytes_read > 0 && buffer[bytes_read - 1] == '\n')
            {
                buffer[bytes_read - 1] = '\0';
                bytes_read--;
            }
            else
            {
                buffer[bytes_read] = '\0';
            }
        }

        // For interactive mode only
        if (interactive_mode)
        {
            if (strcmp(buffer, "exit") == 0)
            {
                print_goodbye();
                break;
            }
            parse_and_execute(buffer, interactive_mode);
        }
    }

    if (input_file)
    {
        fclose(input_file);
    }

    return 0;
}

void print_welcome()
{
    printf("Welcome to my shell!\n");
}

void print_goodbye()
{
    printf("Exiting my shell.\n");
}

void parse_and_execute(char *command, int interactive_mode)
{
    char *input_file = NULL;
    char *output_file = NULL;
    char *args[BUFFER_SIZE];
    char *left_command[BUFFER_SIZE];
    char *right_command[BUFFER_SIZE];
    int is_pipeline = 0;
    int arg_count = 0;
    int right_arg_count = 0;
    int total_quote_count = 0;

    // Count total quotes in the command
    for (char *ptr = command; *ptr != '\0'; ptr++)
    {
        if (*ptr == '"')
        {
            total_quote_count++;
        }
    }

    char *token = strtok(command, " ");

    while (token != NULL)
    {
        if (strcmp(token, "<") == 0 && total_quote_count % 2 == 0)
        {
            // Input redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                input_file = token;
            }
            else
            {
                fprintf(stderr, "Syntax error: expected input file after '<'\n");
                return;
            }
        }
        else if (strcmp(token, ">") == 0 && total_quote_count % 2 == 0)
        {
            // Output redirection
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                output_file = token;
            }
            else
            {
                fprintf(stderr, "Syntax error: expected output file after '>'\n");
                return;
            }
        }
        else if (strcmp(token, "|") == 0 && total_quote_count % 2 == 0)
        {
            // Pipeline
            is_pipeline = 1;
            args[arg_count] = NULL; // End left command arguments

            // Copy args to left_command
            for (int i = 0; i < arg_count; i++)
            {
                left_command[i] = args[i];
            }
            left_command[arg_count] = NULL;

            // Process right command arguments
            right_arg_count = 0;
            token = strtok(NULL, " ");
            while (token != NULL && strcmp(token, ">") != 0)
            {
                right_command[right_arg_count++] = token;
                token = strtok(NULL, " ");
            }

            // Handle output redirection after pipe if present
            if (token != NULL && strcmp(token, ">") == 0)
            {
                token = strtok(NULL, " ");
                if (token != NULL)
                {
                    output_file = token;
                }
                else
                {
                    fprintf(stderr, "Syntax error: expected output file after '>'\n");
                    return;
                }
            }

            right_command[right_arg_count] = NULL;
            break;
        }
        else
        {
            // Regular argument
            args[arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    // Expand wildcards in the left command arguments
    expand_wildcards(args, &arg_count);

    // Add the built-in command check HERE, before pipeline and executable checks
    // printf("\n%s\n", args[0]);
    if (is_builtin_command(args[0]))
    {
        execute_builtin_command(args);
    }
    // If there's a pipeline, expand wildcards in the right command arguments
    else if (is_pipeline)
    {
        expand_wildcards(right_command, &right_arg_count);
        handle_pipeline(left_command, right_command, interactive_mode, output_file);
    }
    else
    {
        // Search for the executable in predefined directories if it's not a path
        char *executable_path = search_executable(args[0]);
        if (executable_path != NULL)
        {
            args[0] = executable_path;
            handle_redirection_and_execute(args, input_file, output_file, interactive_mode);
            free(executable_path);
        }
        else
        {
            fprintf(stderr, "%s: Command not found\n", args[0]);
        }
    }

    // Reset redirection state after each command
    input_file = NULL;
    output_file = NULL;
}

void expand_wildcards(char **args, int *arg_count)
{
    glob_t glob_result;
    int new_arg_count = 0;
    char *expanded_args[BUFFER_SIZE];

    for (int i = 0; i < *arg_count; i++)
    {
        if (strchr(args[i], '*') != NULL)
        {
            // Expand wildcard
            glob(args[i], GLOB_TILDE, NULL, &glob_result);
            if (glob_result.gl_pathc > 0)
            {
                for (size_t j = 0; j < glob_result.gl_pathc; j++)
                {
                    expanded_args[new_arg_count++] = strdup(glob_result.gl_pathv[j]);
                }
            }
            else
            {
                // No match found, keep the original token
                expanded_args[new_arg_count++] = strdup(args[i]);
            }
            globfree(&glob_result);
        }
        else
        {
            // No wildcard, keep the original token
            expanded_args[new_arg_count++] = strdup(args[i]);
        }
    }

    // Copy expanded arguments back to args
    for (int i = 0; i < new_arg_count; i++)
    {
        args[i] = expanded_args[i];
    }
    args[new_arg_count] = NULL;
    *arg_count = new_arg_count;
}

int is_builtin_command(char *command)
{
    // printf("COMMAND : %s Waflkasjfkjhasf", command);
    return (strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0 ||
            strcmp(command, "which") == 0 || strcmp(command, "exit") == 0);
}

void execute_builtin_command(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL || args[2] != NULL)
        {
            fprintf(stderr, "cd: Invalid number of arguments\n");
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                perror("cd");
            }
        }
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("pwd");
        }
    }
    else if (strcmp(args[0], "which") == 0)
    {
        if (args[1] == NULL || args[2] != NULL)
        {
            fprintf(stderr, "which: Invalid number of arguments\n");
        }
        else
        {
            char *executable_path = search_executable(args[1]);
            if (executable_path != NULL)
            {
                printf("%s\n", executable_path);
                free(executable_path);
            }
        }
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        for (int i = 1; args[i] != NULL; i++)
        {
            printf("%s ", args[i]);
        }
        printf("\n");
        exit(0);
    }
}

char *search_executable(char *command)
{
    if (strchr(command, '/'))
    {
        // Command contains a slash, assume it's a path
        if (access(command, X_OK) == 0)
        {
            return strdup(command);
        }
        else
        {
            return NULL;
        }
    }

    // Directories to search
    const char *directories[SEARCH_PATHS] = {"/usr/local/bin", "/usr/bin", "/bin"};
    for (int i = 0; i < SEARCH_PATHS; i++)
    {
        char path[BUFFER_SIZE];
        snprintf(path, sizeof(path), "%s/%s", directories[i], command);
        if (access(path, X_OK) == 0)
        {
            // printf("\nPATH: %s\n", path);
            return strdup(path);
        }
    }

    return NULL;
}

void handle_redirection_and_execute(char **args, char *input_file, char *output_file, int interactive_mode)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        return;
    }

    if (pid == 0)
    {
        // Child process
        // Handle input redirection
        if (input_file != NULL)
        {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0)
            {
                fprintf(stderr, "Error opening input file '%s': %s\n",
                        input_file, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(fd_in, STDIN_FILENO) < 0)
            {
                perror("Error redirecting input");
                close(fd_in);
                exit(EXIT_FAILURE);
            }
            close(fd_in);
        }

        // Handle output redirection
        if (output_file != NULL)
        {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (fd_out < 0)
            {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("Execution failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if (interactive_mode && exit_code != 0)
            {
                printf("Command failed with code %d\n", exit_code);
            }
        }
        else if (WIFSIGNALED(status))
        {
            int signal_number = WTERMSIG(status);
            if (interactive_mode)
            {
                printf("Terminated by signal: %d\n", signal_number);
            }
        }
    }
}

void handle_pipeline(char **left_command, char **right_command, int interactive_mode, char *output_file)
{
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("Pipe failed");
        return;
    }

    pid1 = fork();
    if (pid1 < 0)
    {
        perror("Fork failed");
        return;
    }

    if (pid1 == 0)
    {
        // First child: execute left command
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp(left_command[0], left_command);
        perror("Execution failed");
        exit(EXIT_FAILURE);
    }

    pid2 = fork();
    if (pid2 < 0)
    {
        perror("Fork failed");
        return;
    }

    if (pid2 == 0)
    {
        // Second child: execute right command
        close(pipefd[1]); // Close unused write end
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Handle output redirection if specified
        if (output_file != NULL)
        {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (fd_out < 0)
            {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(right_command[0], right_command);
        perror("Execution failed");
        exit(EXIT_FAILURE);
    }

    // Parent process: close both ends of the pipe and wait for children
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
