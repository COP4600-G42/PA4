#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 1024

int deviceRead(char *message, char *messageLength)
{
    int outputDevice = open("/dev/pa4_output", O_RDWR);
    int returnValue, receiveLength;

    if (outputDevice < 0)
    {
        perror("TESTPA4: Failed to open the output device.\n");

        return errno;
    }

    message = calloc(BUFFER_LENGTH, sizeof(char));

    printf("TESTPA4: How many bytes would you like to read?\n");
    printf("TESTPA4: %s", message);

    fgets(messageLength, BUFFER_LENGTH, stdin);
    receiveLength = atoi(messageLength);

    returnValue = read(outputDevice, message, receiveLength);

    printf("TESTPA4: %s\n", message);

    free(message);
    message = NULL;

    close(outputDevice);
}

int deviceWrite(char *stringToSend)
{
    int inputDevice  = open("/dev/pa4_input", O_RDWR);
    int returnValue, stringLength;

    if (inputDevice < 0)
    {
        perror("TESTPA4: Failed to open the input device.\n");

        return errno;
    }

    printf("TESTPA4: Enter a string to write:\n");
    printf("TESTPA4: ");

    fgets(stringToSend, BUFFER_LENGTH, stdin);
    strtok(stringToSend, "\n");
    stringLength = strlen(stringToSend) >= BUFFER_LENGTH ? BUFFER_LENGTH : strlen(stringToSend);

    returnValue = write(inputDevice, stringToSend, stringLength);

    close(inputDevice);
}

int main(void)
{
    char inputChoice, c, receiveLength;
    char *stringToSend, *messageLength, *message;
    char *ptr;

    stringToSend  = NULL;
    messageLength = NULL;
    message       = NULL;

    stringToSend  = malloc(sizeof(char) * BUFFER_LENGTH);
    messageLength = malloc(sizeof(char) * BUFFER_LENGTH);

    printf("TESTPA4: Type W to write.\n");
    printf("TESTPA4: Type R to read.\n");
    printf("TESTPA4: Type Q to quit.\n");

    do {
        printf("TESTPA4: What do you want to do? ");
        inputChoice = getchar();

        // Clear the stdin buffer
        while ((c = getchar()) != '\n' && c != EOF);

        switch (inputChoice)
        {
            case 'w':
            case 'W':
                deviceWrite(stringToSend);
                break;
            case 'r':
            case 'R':
                deviceRead(message, messageLength);
                break;
            case 'q':
            case 'Q':
                printf("TESTPA4: Bye-bye.\n");
                break;
            default:
                printf("TESTPA4: Invalid input option.\n");
                break;
        }
    } while (inputChoice != 'Q' && inputChoice != 'q');

    free(stringToSend);
    free(messageLength);

    if (message != NULL)
    {
        free(message);
    }

    return 0;
}
