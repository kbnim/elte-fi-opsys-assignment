#ifndef Application_H
#define Application_H

#include <sys/types.h>

#include "Vector.h"

#define FILENAME "./poems.txt"
#define MAX_NUMBER_OF_CHILDREN 4
#define PROGRAM_NAME_MAX_LENGTH 1024

typedef enum Command {
    NO_COMMAND,
    INSERT,
    LIST,
    SPRINKLE,
    HELP,
    SAVE,
    QUIT,
    EDIT,
    REMOVE,
    ERROR
} Command;

typedef unsigned long Argument;
#define NO_ARGUMENTS (Argument)0

typedef struct ApplicationCommand {
    Command command;
    Argument argument;
} ApplicationCommand;

/* Type definition of 'Application'. */
typedef struct Application {
    FILE *file;
    Vector *vector;
    bool quit_state;
    bool is_edited;
    ApplicationCommand command_to_execute;
    char program_name[PROGRAM_NAME_MAX_LENGTH];
} Application;

/* Initialises the 'Application' object. */
void application_initialise(Application* const application, const char* const name);

/*
  Runs (executes) the application. 
  Returns 0 if the application executed successfully.
  Otherwise, a non-zero value is returned.
*/
int application_run(Application* application);

#endif // Application_H