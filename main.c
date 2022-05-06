#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


struct Command {
    // Structure to store info about a user's entered command
    char* commandName;
    int totalArguments;
    char* arguments[512];
    bool input_redirect;
    char* input_redirect_name;
    bool output_redirect;
    char* output_redirect_name;
    bool sendToBackground;
};


void SIGINT_handler(int signalNumber){
    char* message = "Caught SIGINT!\n";
    write(STDOUT_FILENO, message, 15);
}


void readLine(char* inputBuffer, const char* pidString, int pidStringLength) {
    // Reads in a line of input from stdin to inputBuffer that is a maximum of 2048 characters
    int index = 0;

    while (index < 2048) {
        int currentChar = fgetc(stdin);
        if (currentChar == EOF || currentChar == '\n') break;

        // If we run into a $, check to see if next character is also $
        if (currentChar == '$') {
            // Break out if the current char is the last possible (2047th index)
            if (index + 1 == 2048) break;

            // Otherwise, check the next character to see if it is also '$'
            int nextChar = fgetc(stdin);
            if (nextChar == EOF || nextChar == '\n') break;

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
    }
}


int main(int argc, char* argv[]) {
    // Ensure that the user doesn't provide smallsh any arguments to start the shell
    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    char inputBuffer[2048];

    // Initialize struct to prevent SIGINT (Ctrl-C) from terminating smallsh
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIGINT_handler;
    // Block all catchable signals while signal handler is running
    sigfillset(&SIGINT_action.sa_mask);
    // No flags
    SIGINT_action.sa_flags = 0;
    // Install signal handler
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Get the PID for the smallsh instance in string form for variable expansion
    pid_t shellPid = getpid();
    int pidStringLength = snprintf(NULL, 0, "%d", shellPid);
    char pidString[pidStringLength];
    sprintf(pidString, "%d", shellPid);



    while (1) {
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
            } else if (strcmp(token, "&") == 0) {
                token = strtok(NULL, " ");
                if (token == NULL) {
                    // & is last argument, this command should go to background
                    userCommand.sendToBackground = true;
                    break;
                } else {
                    // & found but not last argument, should be treated as regular argument
                    userCommand.arguments[argumentIndex] = "&";
                    userCommand.arguments[argumentIndex + 1] = token;
                    userCommand.totalArguments += 2;
                    argumentIndex += 2;
                }
            } else {
                userCommand.arguments[argumentIndex] = token;
                userCommand.totalArguments++;
                argumentIndex++;
            }
        }

        // Make sure the last argument is NULL so exec will work
        userCommand.arguments[argumentIndex] = NULL;
        userCommand.totalArguments++;

        // TESTING - Print out the contents of the userCommand struct----------------------------------
        printf("userCommand.commandName = %s\n", userCommand.commandName);

        printf("userCommand.totalArguments = %d\n", userCommand.totalArguments);
        printf("userCommand.arguments = ");
        for (int i = 0; i < userCommand.totalArguments; i++) {
            printf("%s ", userCommand.arguments[i]);
        }
        printf("\n");

        if (userCommand.input_redirect == true) {
            printf("userCommand.input_redirect = true\n");
            printf("userCommand.input_redirect_name = %s\n", userCommand.input_redirect_name);
        } else {
            printf("userCommand.input_redirect = false\n");
        }

        if (userCommand.output_redirect == true) {
            printf("userCommand.output_redirect = true\n");
            printf("userCommand.output_redirect_name = %s\n", userCommand.output_redirect_name);
        } else {
            printf("userCommand.output_redirect = false\n");
        }

        printf("userCommand.sendToBackground = %s\n", userCommand.sendToBackground ? "true" : "false");
        // --------------------------------------------------------------------------------------------


        // TODO - Need to clean up all child processes here before breaking out?
        if (strcmp(userCommand.commandName, "exit") == 0) break;
    }

    // Read in a line of user input one char at a time
    // Separate line of user input into the command struct to keep track of:
    //      1. command name
    //      2. array of arguments
    //      3. < input redirect file name (if any; NULL if none)
    //      4. > output redirect file name (if any; NULL if none)

    return EXIT_SUCCESS;
}