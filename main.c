#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <sys/wait.h>


bool childProcessRun = false;
int lastForegroundExitStatus = 0;


struct Command {
    // Structure to store info about a user's entered command
    char* commandName;
    int totalArguments;
    char* arguments[513];
    bool input_redirect;
    char* input_redirect_name;
    bool output_redirect;
    char* output_redirect_name;
    bool sendToBackground;
};


void status(void) {

}


void readLine(char* inputBuffer, const char* pidString, int pidStringLength) {
    // Reads in a line of input from stdin to inputBuffer that is a maximum of 2048 characters
    int index = 0;

    int currentChar;
    currentChar = fgetc(stdin);

    while (currentChar != EOF) {
        if (currentChar == '\n') return;

        // If we run into a $, check to see if next character is also $
        if (currentChar == '$') {
            // Break out if the current char is the last possible (2047th index)
            if (index + 1 == 2048) return;

            // Otherwise, check the next character to see if it is also '$'
            int nextChar = fgetc(stdin);
            if (nextChar == '\n') return;
            if (nextChar == EOF) break;

            if (nextChar == '$') {
                // variable expansion (add pidString to inputBuffer)
                for (int i = 0; i < pidStringLength; i++) {
                    inputBuffer[index] = pidString[i];
                    index++;
                }
            } else {
                // no variable expansion (add currentChar and nextChar to inputBuffer)
                inputBuffer[index] = currentChar;
                inputBuffer[index + 1] = nextChar;
                index += 2;
            }
        } else {
            inputBuffer[index] = currentChar;
            index++;
        }

        currentChar = fgetc(stdin);
    }

    // If EOF actually found, exit with an error. If user did Ctrl-C, clear error and this function will
    // return. In main, we will loop back again and read another line of input from the user.
    if (feof(stdin)) {
        fprintf(stderr, "\nfgetc() found EOF; stdin must have been closed; exiting\n");
        exit(1);
    } else if (ferror(stdin)) {
        clearerr(stdin);
    }
}


int main(int argc, char* argv[]) {
    // Ensure that the user doesn't provide smallsh any arguments to start the shell
    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    char inputBuffer[2048];

    pid_t childPids[500];
    memset(childPids, 0, sizeof(childPids));
    int childPidsCount = 0;

    pid_t backgroundPid;
    int backgroundProcessExitStatus = 0;

    // Setup ignore_action to ignore SIGINT (Ctrl-C)
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);

    // Get the PID for the smallsh instance in string form for variable expansion
    pid_t shellPid = getpid();
    int pidStringLength = snprintf(NULL, 0, "%d", shellPid);
    char pidString[pidStringLength];
    sprintf(pidString, "%d", shellPid);


    while (1) {
        // Before re-prompting, have a while loop go forever until waitpid waiting for
        // any process returns 0 (meaning no zombie processes to clean up at the moment,
        // and then re-prompt the user)
        while(childPidsCount > 0) {
            // printf("childPidsCount is %d which is > than 0, checking if any have finished before re-prompt\n", childPidsCount);

            backgroundPid = waitpid(-1, &backgroundProcessExitStatus, WNOHANG);

            // ("backgroundPid is: %d\n", backgroundPid);

            if (backgroundPid == 0) {
                // no child process is complete, break out so we can re-prompt the user for a command
                break;
            } else if (backgroundPid == -1) {
                // err has occurred, exit smallsh?
                err(errno, "waitpid()");
            } else {
                // child process has completed, print out appropriate message and loop back around
                if (WIFEXITED(backgroundProcessExitStatus)) {
                    printf("background pid %d is done: exit value %d\n",
                           backgroundPid, WEXITSTATUS(backgroundProcessExitStatus));
                } else {
                    printf("background pid %d is done: terminated by signal %d\n",
                           backgroundPid, WTERMSIG(backgroundProcessExitStatus));
                }
                fflush(stdout);
                childPidsCount--;
            }
        }

        // Prompt user for a command
        printf(": ");
        fflush(stdout);

        // Read in a line of user input
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        readLine(inputBuffer, pidString, pidStringLength);

        if (inputBuffer[0] == '#' || inputBuffer[0] == '\0' || inputBuffer[0] == '\n') continue;

        // Declare a struct to store the user's command and initialize defaults
        struct Command userCommand;
        userCommand.totalArguments = 0;
        userCommand.input_redirect = false;
        userCommand.output_redirect = false;
        userCommand.sendToBackground = false;

        char* token;
        int argumentIndex = 0;

        // strtok the inputBuffer and break it up by empty spaces?
        // first strtok gets you command
        token = strtok(inputBuffer, " ");
        if (token == NULL) {
            continue;
        }
        userCommand.commandName = token;
        userCommand.arguments[argumentIndex] = token;
        userCommand.totalArguments++;
        argumentIndex++;


        // then keep strtok until you get <, >, &, or NULL
        while (1) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                break;
            } else if (strcmp(token, "<") == 0) {
                token = strtok(NULL, " ");
                userCommand.input_redirect = true;
                userCommand.input_redirect_name = token;
            } else if (strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                userCommand.output_redirect = true;
                userCommand.output_redirect_name = token;
            } else {
                userCommand.arguments[argumentIndex] = token;
                userCommand.totalArguments++;
                argumentIndex++;
            }
        }

        if (userCommand.totalArguments - 1 >= 0 && strcmp(userCommand.arguments[userCommand.totalArguments - 1], "&") == 0) {
            // Last argument is &, make sendToBackground true if command is not exit, cd, or status
            if ( strcmp(userCommand.arguments[0], "exit") != 0 &&
                 strcmp(userCommand.arguments[0], "cd") != 0 &&
                 strcmp(userCommand.arguments[0], "status") != 0) {
                userCommand.sendToBackground = true;
            }
            // Replace the last argument (&) with NULL
            userCommand.arguments[userCommand.totalArguments - 1] = NULL;
        } else {
            // Otherwise, make sure the last argument is NULL so exec will work
            userCommand.arguments[argumentIndex] = NULL;
            userCommand.totalArguments++;
        }


        // TESTING - Print out the contents of the userCommand struct----------------------------------
//        printf("userCommand.commandName = %s\n", userCommand.commandName);
//
//        printf("userCommand.totalArguments = %d\n", userCommand.totalArguments);
//        printf("userCommand.arguments = ");
//        for (int i = 0; i < userCommand.totalArguments; i++) {
//            printf("%s ", userCommand.arguments[i]);
//        }
//        printf("\n");
//
//        if (userCommand.input_redirect == true) {
//            printf("userCommand.input_redirect = true\n");
//            printf("userCommand.input_redirect_name = %s\n", userCommand.input_redirect_name);
//        } else {
//            printf("userCommand.input_redirect = false\n");
//        }
//
//        if (userCommand.output_redirect == true) {
//            printf("userCommand.output_redirect = true\n");
//            printf("userCommand.output_redirect_name = %s\n", userCommand.output_redirect_name);
//        } else {
//            printf("userCommand.output_redirect = false\n");
//        }
//
//        printf("userCommand.sendToBackground = %s\n", userCommand.sendToBackground ? "true" : "false");
        // --------------------------------------------------------------------------------------------


        if (strcmp(userCommand.commandName, "cd") == 0) {
            char cur_dir[FILENAME_MAX];
            getcwd(cur_dir, sizeof(cur_dir));
            printf("cwd is: %s\n", cur_dir);
            fflush(stdout);

            // If user provided no arguments, chdir to $HOME, otherwise chdir to path provided
            if (userCommand.arguments[1] == NULL) {
                if(chdir(getenv("HOME")) != 0) err(errno, "chdir()");
                printf("path name is NULL. changing directory to $HOME\n");
                fflush(stdout);
            } else {
                if(chdir(userCommand.arguments[1]) != 0) err(errno, "chdir()");
                printf("path name is not NULL. changing directory to %s\n", userCommand.arguments[1]);
                fflush(stdout);
            }

            getcwd(cur_dir, sizeof(cur_dir));
            printf("cwd is: %s\n", cur_dir);
            fflush(stdout);

        } else if (strcmp(userCommand.commandName, "exit") == 0) {
            // Exits smallsh. Takes no arguments. The shell will kill any other processes
            // or jobs that the shell started before it terminates itself.

            // Loop through the childPids and kill each one and wait on it
            for (int i = 0; i < childPidsCount; i++ ) {
                // printf("killing child pid %d\n", childPids[i]);

                // kill process with pid at childPids[i]
                kill(childPids[i], SIGKILL);

                int zombieStatus;

                // wait on process with pid at childPids[i]
                waitpid(childPids[i], &zombieStatus, 0);
            }

            break;
        } else if (strcmp(userCommand.commandName, "status") == 0) {
            // Prints exit status or the terminating signal of the last foreground
            // ran by smallsh. If run before any foreground command is run, the exit
            // status of 0 is printed. Built-in shell commands cd, exit, and status
            // are not counted as foreground processes and won't update status.
            if (childProcessRun) {
                // interpret lastForegroundExitStatus
                if (WIFEXITED(lastForegroundExitStatus)) {
                    // print exit status
                    printf("exit value %d\n", WEXITSTATUS(lastForegroundExitStatus));
                    fflush(stdout);
                } else {
                    // print signal number
                    printf("terminated by signal %d\n", WTERMSIG(lastForegroundExitStatus));
                    fflush(stdout);
                }
            } else {
                printf("exit value 0\n");
                fflush(stdout);
            }

        } else {
            // spawn a child
            int childStatus;

            pid_t spawnpid = fork();

            // TODO - remove alarm after testing/debugging
            alarm(60);

            switch (spawnpid) {
                case -1:
                    perror("fork() failed!");
                    exit(1);

                case 0:
                    // child - execute command, with arguments, in fg or bg
                    // TODO - handle input and output redirection using dup2()
                    execvp(userCommand.commandName, userCommand.arguments);

                    fprintf(stderr, "%s: ", userCommand.commandName);
                    fflush(stderr);
                    perror("");
                    exit(EXIT_FAILURE);

                default:
                    //parent
                    // foreground? wait; background? return immediately
                    if (userCommand.sendToBackground) {
                        // don't wait for child
                        printf("background pid is %d\n", spawnpid);
                        fflush(stdout);

                        // add child's PID to array to later be waited on before re-prompting user?
                        // probably do the signal method?
                        if (childPidsCount >= 500) err(errno=EAGAIN, "child processes full");

                        // Find an empty slot in the childPids array to put new background process
                        int index = 0;
                        while (index < 500 && childPids[index] != 0) {
                            index++;
                        }
                        if (index < 500 && childPids[index] == 0) {
                            childPids[index] = spawnpid;
                            childPidsCount++;
                        }

                        spawnpid = waitpid(spawnpid, &childStatus, WNOHANG);

                    } else {
                        // foreground processes must be waited on
                        childProcessRun = true;
                        spawnpid = wait(&lastForegroundExitStatus);
                    }

                    break;
            }
        }



        // TODO - Need to clean up all child processes here before breaking out?
        if (strcmp(userCommand.commandName, "exit") == 0) break;
    }

    return EXIT_SUCCESS;
}