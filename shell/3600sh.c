/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

// For comparator operators
#define TRUE 0
#define FALSE 1

#define USE(x) (x) = (x)
#define MAX 1024

static void execute(int argc, char *argv[]);
static void getargs(char command[], int *argcp, char *argv[], int count);
static char * getword(char * begin, char **end_p, int * count, char * word, int * redirection_seen);
static void prompt();
void do_exit();
static void reset();

int losc[4] = { FALSE, FALSE, FALSE, FALSE };
/* losc[0] = Redirection to output file
 * losc[1] = Redirection from input file
 * losc[2] = Redirection to ouput file from stderr
 * losc[3] = Background process
 */

int error_types[3] = { FALSE, FALSE, FALSE };
/* error_types[0] = Unable to open redirection file.
 * error_types[1] = Unrecognized escape sequence.
 * error_types[2] = Invalid syntax.
 */

// Holds the stdin, stdout, and stderr file names
char std_files[3][MAX];


// Makes sure we do not output more than one error
int do_not_execute = FALSE;

int main(int argc, char *argv[]) {

    // Code which sets stdout to be unbuffered
    // This is necessary for testing; do not change these lines
    USE(argc);
    USE(argv);
    setvbuf(stdout, NULL, _IONBF, 0);

    int newline_b = FALSE;

    prompt();

    while(1) {

        char * command = (char *) calloc(MAX, sizeof(char));
        char ** childargv = (char **) calloc(MAX, sizeof(char *));
        int childargc = 0;
        int c = getc(stdin);

        // Keeps track of how long the command is
        int count = 0;

        // Keeps track of whether we need to realloc
        int realloc_count = 0;

        // Print prompt after newline but not EOF
        if(newline_b == TRUE) {
            prompt();
            newline_b = FALSE;
        }

        // Exit without printing prompt
        if(c == EOF) {

            // Free everything
            free(command);
            free(childargv);
            for(int i = 0; i < childargc; i++) {
                free(childargv[i]);
            }

            do_exit();
        }

        // Splits the input into first command and then the rest
        // Completely re-loops on the second part of the input in case
        // there are multiple newlines
        while(1) {

            realloc_count = count % (MAX - 1);
            if(realloc_count > 0) {
                command = realloc(command, MAX);
            }

            if (c == EOF) {
                command[count] = '\0';
                break;
            } else if (c == '\n') {
                newline_b = TRUE;
                command[count] = '\0';
                break;
            } else {
                command[count] = c;
                count++;
                c = getc(stdin);
            }

        }

        if (do_not_execute == FALSE) {

            // Gets the argument list
            getargs(command, &childargc, childargv, count);
            if(error_types[1] == TRUE) {
                fprintf(stderr, "Error: Unrecognized escape sequence.\n");
                do_not_execute = TRUE;
            } else if(error_types[2] == TRUE) {
                fprintf(stderr, "Error: Invalid syntax.\n");
                do_not_execute = TRUE;
            } else {
                execute(childargc, childargv);
            }
        }

        // Resets the variables used in execution
        reset();

    }
}

// Executes the command
static void execute(int argc, char *argv[]) {

    USE(argc);

    pid_t child_pid;

    child_pid = fork();

    // Error occured while forking
    if(child_pid == -1) {

        printf("An error occured in fork");

        // The child enters here, and the parent waits
    } else if(child_pid == 0) {

        // handles >
        if(losc[0] == TRUE) {
            int fd = open(std_files[0], O_RDWR | O_CREAT, S_IRUSR, S_IWUSR | O_TRUNC);
            if (fd < 0) {
                error_types[0] = TRUE;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // handles <
        if(losc[1] == TRUE) {
            int fd = open(std_files[1], O_RDONLY | S_IRUSR | S_IWUSR | O_TRUNC);
            if (fd < 0) {
                error_types[0] = TRUE;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // handles 2>
        if(losc[2] == TRUE) {
            int fd = open(std_files[2], O_WRONLY | S_IRUSR | S_IWUSR | O_TRUNC);
            if (fd < 0) {
                error_types[0] = TRUE;
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        // handles &
        if(losc[3] == TRUE) {
            //Set the child in its own process group
            setpgid(0,0);
        }

        // Check for error before execution
        if(error_types[0] == TRUE) {
            fprintf(stderr, "Error: Unable to open redirection file.\n");
            do_not_execute = TRUE;
            exit(0);
        } else {

            // Child process executes the command, error if -1
            if (execvp(argv[0], argv) == -1) {

                // The child had an error, print it out
                if(errno == ENOENT) {
                    fprintf(stderr, "Error: Command not found.\n");
                } else {
                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                }

                do_not_execute = TRUE;

                // Exit the child
                exit(0);
            }
        }

    } else {

        //Don't wait if it's a background process
        if (losc[3] != TRUE) {
            if(child_pid != 0) {

                // The parent waits if it is a normal process
                waitpid(child_pid, NULL, 0);
            }
        }

    }
}

static void getargs(char command[], int *argcp, char *argv[], int count) {

    char *end;
    int i = 0;
    int index = 0;

    // Keeps track of redirection cases for error handling
    int redirection_seen[2] = { FALSE, FALSE };

    // When the command is exhausted
    while(index < count) {

        int realloc_count = *argcp % (MAX - 1);
        if(realloc_count > 0) {
            argv = realloc(argv, MAX);
        }

        char * word = (char *) calloc(MAX, sizeof(char));
        getword(command, &end, &index, word, redirection_seen);

        // Error handling for background processes
        if(losc[3] == TRUE) {

            char * word_test = (char *) calloc(MAX, sizeof(char));
            command = end;
            getword(command, &end, &index, word_test, redirection_seen);

            if(*word_test != '\0') {
                error_types[2] = TRUE;
                free(word_test);
                return;

            } else {

                argv[i] = NULL; // Safety NULL
                *argcp = i;
                free(word_test);
                return;
            }
        }

        // Handles output redirection and error handling
        if (*command == '>') {

            losc[0] = TRUE;
            command = end + 1;
            index++;

            if(index > count) {
                error_types[2] = TRUE;
            }

            redirection_seen[1] = TRUE;

            // Insures segfault doesn't happen
            char toCheck[MAX];
            getword(command, &end, &index, toCheck, redirection_seen);
            if(strncmp(toCheck, "", 1) != 0) {
                if (strncat(std_files[0], toCheck, MAX) == NULL ) {
                    printf("No stdout_file given");
                }
            }

            std_files[0][MAX - 1] = '\0';

            redirection_seen[1] = FALSE;
            redirection_seen[0] = TRUE;

            command = end + 1;
            index++;

        // Handles input redirection and error handling
        } else if (*command == '<') {

            losc[1] = TRUE;
            command = end + 1;
            index++;

            if(index > count) {
                error_types[2] = TRUE;
            }

            redirection_seen[1] = TRUE;

            // Insures segfault doesn't happen
            char toCheck[MAX];
            getword(command, &end, &index, toCheck, redirection_seen);
            if(strncmp(toCheck, "", 1) != 0) {
                if (strncat(std_files[1], toCheck, MAX) == NULL ) {
                    printf("No stdin_file given");
                }
            }

            std_files[1][MAX - 1] = '\0';

            redirection_seen[1] = FALSE;
            redirection_seen[0] = TRUE;

            command = end + 1;
            index++;

        // Handles error redirection and error handling
        } else if (strncmp(command, "2>", 3) == 0) {

            losc[2] = TRUE;
            command = end + 1;
            index++;

            if(index > count) {
                error_types[2] = TRUE;
            }

            redirection_seen[1] = TRUE;

            // Insures segfault doesn't happen
            char toCheck[MAX];
            getword(command, &end, &index, toCheck, redirection_seen);
            if(strncmp(toCheck, "", 1) != 0) {
                if (strncat(std_files[2], toCheck, MAX) == NULL ) {
                    printf("No stderr_file given");
                }
            }

            std_files[2][MAX - 1] = '\0';

            redirection_seen[1] = FALSE;
            redirection_seen[0] = TRUE;

            command = end + 1;
            index++;

        } else {

            // Do not add an empty word
            if(strncmp(word, "", 1) == 0) {
                argv[i] = NULL; // Safety NULL
                *argcp = i;
                return;
            }

            // Already converted to a null terminated string
            argv[i] = word;
            i++;

            // To get to the start of the next word or EOF
            command = end + 1;
            index++;

        }
    }

    argv[i] = NULL; // Safety NULL
    *argcp = i - 1;
}

static char * getword(char * begin, char **end_p, int * count, char * word, int * redirection_seen) {
    char * end = begin;
    int i = 0;

    while ( *begin == ' ' || *begin == '\t' ) {
        begin++;  // Get rid of leading spaces and tabs
        (*count)++;
    }

    end = begin;

    while (*end != '\0' && *end != ' ' && *end != '\n' && *end != '\t') {

        // Handles background
        if(*end == '&') {
            end++;
            (*count)++;

            if(losc[3] == TRUE) {
                error_types[2] = TRUE;
            }

            losc[3] = TRUE;

            word[i] = '\0'; // Add terminating string
            *end_p = end;

            return word;
        }

        // Special characters using escape sequences
        if(*end == '\\') {

            end++;
            (*count)++;

            if(*end == '\\' || *end == '&' || *end == ' ' || *end == 't') {

                if(*end == 't') {

                    word[i] = '\t';

                    i++;
                    end++;
                    (*count)++;

                } else {

                    word[i] = *end;

                    i++;
                    end++;
                    (*count)++;

                }

            } else {

                // Invalid syntax
                error_types[1] = TRUE;
                return NULL;

            }

        } else {

            word[i] = *end;
            i++;
            end++;
            (*count)++;

        }
    }

    if ( end == begin ) {
        return NULL;  // No more words
    }

    word[i] = '\0'; // Add terminating string
    *end = '\0';  // Add terminating string
    *end_p = end;

    // Special cases for redirection files
    if(redirection_seen[1] == TRUE) {
        if ((strncmp(word, "<", 2) == 0) || (strncmp(word, ">", 2) == 0) || (strncmp(word, "2>", 3) == 0)) {
            error_types[2] = TRUE;
        }
    }

    if(redirection_seen[0] == TRUE) {
        if (!(strncmp(word, "<", 2) == 0 || strncmp(word, ">", 2) == 0 || strncmp(word, "2>", 3) == 0)) {
            error_types[2] = TRUE;
        }
    }

    redirection_seen[0] = FALSE;
    return word;
}

// Prints the prompt
static void prompt() {

    char * user;
    char host[MAX];
    char cwd[MAX];

    // User
    uid_t uid = geteuid();
    struct passwd * pw = getpwuid(uid);

    if (pw) {
        user = pw->pw_name;
    } else {
        user = "";
    }

    // Host
    if (gethostname(host, sizeof(host)) != 0) {
        printf("Error in gethostname when getting hostname \n");
    }
    host[MAX - 1] = '\0';

    // CWD
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("Error in getcwd when getting current working directory \n");
    }
    cwd[MAX - 1] = '\0';

    printf("%s@%s:%s> ", user, host, cwd);
}

// Prints the goodbye text
void do_exit() {
    printf("So long and thanks for all the fish!\n");

    // Wait for all children to exit, then exit
    while (wait(NULL) > 0) {}
    exit(0);
}

// Resets everything, makes the shell ready for the next command
static void reset() {

    // Reset everything except for the background. It should be last.
    for(unsigned int i = 0; i < (sizeof(losc) - 1) / sizeof(int); i++) {
        losc[i] = FALSE;
    }

    // Reset errors
    for(unsigned int i = 0; i < (sizeof(error_types)) / sizeof(int); i++) {
        error_types[i] = FALSE;
    }

    for(unsigned int index = 0; index < 3; index++) {
        for(unsigned int i = 0; i < MAX; i++) {
            std_files[index][i] = '\0';
        }
    }

    if(losc[3] != TRUE) {
        do_not_execute = FALSE;
    }
}
