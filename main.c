#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


//struct Command {
//    // Structure to store info about a user's entered command
//    char* commandName;
//    char* arguments[512];
//    bool input_redirect;
//    char* input_redirect_name;
//    bool output_redirect;
//    char* output_redirect_name;
//    bool sendToBackground;
//};

void readLine(char* inputBuffer, char* pidString, int pidStringLength) {
    // Reads in a line of input from stdin that is a maximum of 2048 characters
    int index = 0;

    while (1) {
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

//    char* userCommand;
    char inputBuffer[2048];


    // Get the PID for the smallsh instance in string form for variable expansion
    pid_t shellPid = getpid();
    int pidStringLength = snprintf(NULL, 0, "%d", shellPid);
    char pidString[pidStringLength];
    sprintf(pidString, "%d", shellPid);



    // Declare a struct to store the user's command and initialize defaults
//    struct Command userCommand;
//    userCommand.input_redirect = false;
//    userCommand.output_redirect = false;
//    userCommand.sendToBackground = false;



    while (1) {
        // Prompt user for a command
        printf(": ");
        fflush(stdout);

        // Read in a line of user input
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        readLine(inputBuffer, pidString, pidStringLength);

        break;
    }

    int index = 0;

    while (index < 2048 && inputBuffer[index] != EOF) {
        fputc(inputBuffer[index], stdout);
        index++;
    }

    fputc('\n', stdout);


    // Read in a line of user input one char at a time
    // Separate line of user input into the command struct to keep track of:
    //      1. command name
    //      2. array of arguments
    //      3. < input redirect file name (if any; NULL if none)
    //      4. > output redirect file name (if any; NULL if none)

    return EXIT_SUCCESS;
}