#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE 80 
#define DELIMITERS " \t\r\n\a"

char history[MAX_LINE] = "";

void handle_sigint(int sig) {
    printf("\n[shell-we] Use 'exit' to quit.\nshell-we-kimia> ");
    fflush(stdout);
}

char **parse_line(char *line) {
    int bufsize = MAX_LINE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIMITERS);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, DELIMITERS);
    }
    tokens[position] = NULL;
    return tokens;
}

// Function for Pipe Management
void execute_pipe(char *line) {
    char *parts[2];
    parts[0] = strsep(&line, "|");
    parts[1] = line;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe error");
        return;
    }

    pid_t p1 = fork();
    if (p1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        char **args1 = parse_line(parts[0]);
        execvp(args1[0], args1);
        exit(EXIT_FAILURE);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        char **args2 = parse_line(parts[1]);
        execvp(args2[0], args2);
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

int main(void) {
    char line[MAX_LINE];
    char temp_line[MAX_LINE];
    char **args;
    int should_run = 1;

    signal(SIGINT, handle_sigint);

    while (should_run) {
        printf("shell-we-kimia> ");
        fflush(stdout);

        if (fgets(line, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break; 
        }
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        // History management (!!)
        if (strcmp(line, "!!") == 0) {
            if (strlen(history) == 0) {
                printf("No commands in history.\n");
                continue;
            }
            printf("%s\n", history);
            strcpy(line, history);
        } else {
            strcpy(history, line);
        }

        strcpy(temp_line, line);

        // ۱. Checking the Pipe
        if (strchr(temp_line, '|')) {
            execute_pipe(temp_line);
        } else {
            // ۲. Execute normal commands
            args = parse_line(line);

            if (args[0] == NULL) {
                free(args);
                continue;
            }

            if (strcmp(args[0], "exit") == 0) {
                should_run = 0;
            } 
            else if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL) {
                    fprintf(stderr, "shell-we: expected argument to \"cd\"\n");
                } else if (chdir(args[1]) != 0) {
                    perror("shell-we");
                }
            }
            else if (strcmp(args[0], "pwd") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
            }
            else if (strcmp(args[0], "help") == 0) {
                printf("\n--- Shell-We by Kimia Shafaei ---\n");
                printf("Built-ins: cd, exit, pwd, help, !!\n");
                printf("Features: Pipes (|), Background (&), Signals\n\n");
            } 
            else {
                int background = 0;
                int i = 0;
                while (args[i] != NULL) i++;
                if (i > 0 && strcmp(args[i-1], "&") == 0) {
                    background = 1;
                    args[i-1] = NULL;
                }

                pid_t pid = fork();
                if (pid == 0) {
                    if (execvp(args[0], args) == -1) {
                        perror("shell-we execution error");
                    }
                    exit(EXIT_FAILURE);
                } else if (pid < 0) {
                    perror("Fork error");
                } else {
                    if (!background) {
                        waitpid(pid, NULL, 0);
                    } else {
                        printf("[Background PID: %d]\n", pid);
                    }
                }
            }
            free(args);
        }

        // Zombie Clearance
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    return 0;
}