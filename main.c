#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define ZUMI_BUFFER_LEN 5
#define ZUMI_ARGS_LEN   64

char *Zumi_read_line(void);
char **Zumi_parse_args(char *line);
int Zumi_launch(char **args);

int Zumi_cd(char **args);
int Zumi_help(char **args);
int Zumi_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &Zumi_cd,
    &Zumi_help,
    &Zumi_exit
};

int Zumi_num_builtins() { return sizeof(builtin_str) / sizeof(builtin_str[0]); }

int Zumi_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "zumi: expected argument to `cd`\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("zumi");
        }
    }

    return 1;
}

int Zumi_help(char **args) {
    puts("ZUMI Ugly Shell");
    puts("Type command and see failing miserably");
    puts("\nBuiltins commands:");

    for (int i = 0; i < Zumi_num_builtins(); i++) {
        printf("- %s\n", builtin_str[i]);
    }

    return 1;
}

int Zumi_exit(char **args) { return 0; }

char *Zumi_read_line(void) {
    int length = ZUMI_BUFFER_LEN;
    char *buffer = (char *) malloc(length);
    if (!buffer) {
        fprintf(stderr, "ERROR: Zumi_read_line: buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    int position = 0;
    char current;

    while(1) {
        current = getchar();
        if (current == EOF || current == '\n') {
            buffer[position] = 0;
            break;
        }

        if (position >= length) {
            length += ZUMI_BUFFER_LEN;
            buffer = realloc(buffer, length);
            if (!buffer) {
                fprintf(stderr, "ERROR: Zumi_read_line: buffer reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        buffer[position++] = current;
    }

    return buffer;
}

char **Zumi_parse_args(char *line) {
    int argslength = ZUMI_ARGS_LEN;
    char **args = (char **) malloc(sizeof(char *) * argslength);
    if (!args) {
        fprintf(stderr, "ERROR: Zumi_parse_args: args buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *arg;
    int position = 0;

    arg = strtok(line, " ");
    while (arg != NULL) {
        args[position] = arg;
        position++;

        if (position >= argslength) {
            argslength += ZUMI_ARGS_LEN;
            args = realloc(args, argslength * sizeof(char *));
            if (!args) {
                fprintf(stderr, "ERROR: Zumi_parse_args: args buffer reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        arg = strtok(NULL, " ");
    }

    args[position] = NULL;
    return args;
}

int Zumi_execute(char **args) {
    if (args[0] == NULL) return 1;

    for (int i = 0; i < Zumi_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return Zumi_launch(args);
}

int Zumi_launch(char **args) {
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) perror("zumi");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("zumi");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

void Zumi_loop(void) {
    char *line;
    char **args;
    int status = 0;

    do {
        printf("> ");

        line = Zumi_read_line();
        args = Zumi_parse_args(line);
        status = Zumi_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(void) {
    // Initialize

    // Interpret
    Zumi_loop();

    // Finish

    return 0;
}