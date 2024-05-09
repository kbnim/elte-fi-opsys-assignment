#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // srand -- random generator
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> // waidpid
#include <signal.h>
#include <sys/msg.h>  // message queue
#include <sys/sem.h>  // semaphores
#include <sys/stat.h> // semaphore funky macros

#include "Application.h"
#include "PosixUtils.h"
#include "MemoryAllocation.h"

/* Inserts a new poem to the end of the database. */
static void application_command_insert(Application* application);

/* Prints out each element of the database in a formatted way. */
static void application_command_list(Application *application);

/* Funky asynchronous task spedified in Part 2. */
static void application_command_sprinkle(Application *application);

/* Function of the child process related to the 'sprinkle' command. */
static pid_t application_command_sprinkle_child_async(int, int);

/* Prints out each available command and their usage. */
static void application_command_help(Application *application);

/* Saves the contents of the database if it was changes at runtime. If not, it does nothing. */
static void application_command_save(Application *application);

/* Quits the applicaion. Before quitting, it asks whether to save all modifications or not.  */
static void application_command_quit(Application *application);

/* Removes a poem at the specified index. */
static void application_command_remove(Application *application, Argument argument);

/* Edits a poem at the specified index. */
static void application_command_edit(Application *application, Argument argument);

/* Tokenises the input. Returns a vector of strings (i.e. tokens). */
static Vector *application_tokenise_input(const String *const string);

/* Processes the tokens and returns the 'decoded' command. */
static ApplicationCommand application_process_tokens(const Vector *const tokens);

/* Executes the command that is passed in to the function. */
static void application_execute_command(Application *application);

void application_initialise(Application *const application, const char* const name)
{
    // puts("Initialisation in progress.");
    application->file = fopen(FILENAME, "a+");

    if (application->file == NULL)
    {
        perror("Error: opening file failed.");
        exit(-1);
    }

    application->quit_state = false;
    application->is_edited = false;
    application->command_to_execute = (ApplicationCommand){NO_COMMAND, NO_ARGUMENTS};
    strncpy(application->program_name, name, PROGRAM_NAME_MAX_LENGTH);
    application->vector = vector_construct();

    if (application->vector == NULL)
    {
        perror("Error: instantiation of vector failed.");
        exit(-1);
    }

    while (!feof(application->file))
    {
        String *line = string_read_line(application->file);

        if (!string_are_equal_c(line, ""))
        {
            vector_append(application->vector, line);
        }
        else
        {
            // no empty strings are added
            string_destroy(line);
        }
    }

    fclose(application->file);
    application->file = NULL;
}

int application_run(Application* application)
{
    puts("=== Easter Bunny's Poems ===");
    vector_print(application->vector);

    while (!application->quit_state && !feof(stdin))
    {
        printf("> ");
        String* input = string_read_line(stdin);
        Vector* tokens = application_tokenise_input(input);
        application->command_to_execute = application_process_tokens(tokens);
        application_execute_command(application);
        vector_destroy(tokens);
        string_destroy(input);
    }

    vector_destroy(application->vector);
    return application->file != NULL ? fclose(application->file) : EXIT_SUCCESS;
}

static void application_command_insert(Application* application)
{
    printf("Insert new poem > ");
    String* poem = string_read_line(stdin);

    while (string_are_equal_c(poem, "") && !feof(stdin))
    {
        fprintf(stderr, "Invalid input. Try again. > ");
        string_destroy(poem);
        poem = string_read_line(stdin);
    }

    if (!string_are_equal_c(poem, ""))
    {
        vector_append(application->vector, poem);
    }
    else
    {
        // no empty strings are added
        string_destroy(poem);
    }

    if (!application->is_edited)
    {
        application->is_edited = true;
    }
}

static void application_command_list(Application* application)
{
    vector_print(application->vector);
}

static void application_command_sprinkle(Application* application)
{
    // minimal error handling
    if (vector_get_size(application->vector) < 2)
    {
        fprintf(stderr, "Error: not enough poems to select from.\n");
        return;
    }

    if (vector_get_used_count(application->vector) > (vector_get_size(application->vector) - 2))
    {
        fprintf(stderr, "Error: you need at least 2 unused poems in the database.\n");
        return;
    }

    srand(time(NULL));
    int random = random_generator(MAX_NUMBER_OF_CHILDREN, true);

    int child_status;
    pid_t child_process;

    int poem_index_01 = random_generator(vector_get_size(application->vector), false);

    while (string_get_is_used(vector_get_string_at(application->vector, poem_index_01)))
    {
        poem_index_01 = random_generator(vector_get_size(application->vector), false);
    }

    int poem_index_02 = random_generator(vector_get_size(application->vector), false);

    // so that the two indices are different
    while (string_get_is_used(vector_get_string_at(application->vector, poem_index_02)) ||
           poem_index_02 == poem_index_01)
    {
        poem_index_02 = random_generator(vector_get_size(application->vector), false);
    }

    size_t poem_01_size = string_get_size(vector_get_string_at(application->vector, poem_index_01));
    size_t poem_02_size = string_get_size(vector_get_string_at(application->vector, poem_index_02));

    MessageQueue msqueue;
    int msqueue_id;
    int msqueue_status;
    key_t key = ftok(application->program_name, 1);

    char poems[2][MSQUEUE_BUFFER];
    memset(poems[0], 0, MSQUEUE_BUFFER);
    memset(poems[1], 0, MSQUEUE_BUFFER);

    strncpy(poems[0], vector_get_at(application->vector, poem_index_01), poem_01_size);
    strncpy(poems[1], vector_get_at(application->vector, poem_index_02), poem_02_size);
    poems[0][poem_01_size - 1] = '\0';
    poems[1][poem_02_size - 1] = '\0';

    int pipe_io[IO_PORTS];

    if (pipe(pipe_io) < 0)
    {
        perror("Creating pipe failed. :(");
        exit(EXIT_FAILURE);
    }

    msqueue_id = msgget(key, 0600 | IPC_CREAT);

    switch (random)
    {
    case 1:
        child_process = application_command_sprinkle_child_async(pipe_io[RECEIVE], msqueue_id);
        break;
    case 2:
        child_process = application_command_sprinkle_child_async(pipe_io[RECEIVE], msqueue_id);
        break;
    case 3:
        child_process = application_command_sprinkle_child_async(pipe_io[RECEIVE], msqueue_id);
        break;
    case 4:
        child_process = application_command_sprinkle_child_async(pipe_io[RECEIVE], msqueue_id);
        break;
    default:
        break;
    }
    pause();
    write(pipe_io[SEND], poems, 2 * MSQUEUE_BUFFER);
    puts("[PARENT] Poems have been sent");

    msqueue_status = msgrcv(msqueue_id, &msqueue, MSQUEUE_BUFFER, MSQUEUE_TYPE, 0);

    if (msqueue_status < 0)
    {
        perror("Message queue: receiving message failed :(");
    }

    puts("[PARENT] Poem has been received.");

    waitpid(child_process, &child_status, 0);
    close(pipe_io[RECEIVE]);
    close(pipe_io[SEND]);

    size_t used_index =
        string_are_equal_c(vector_get_string_at(application->vector, poem_index_01), msqueue.mtext) ? 
        poem_index_01 : poem_index_02;

    vector_set_used(application->vector, used_index);
}

static pid_t application_command_sprinkle_child_async(int pipe_io, int msqueue_id)
{
    pid_t thread = fork();

    if (thread < 0)
    {
        perror("Fork failed. :(");
        exit(EXIT_FAILURE);
    }
    if (thread > 0)
    {
        return thread;
    }

    kill(getppid(), SIGUSR1);
    srand(getpid());

    char poems[2][MSQUEUE_BUFFER];
    memset(poems[0], 0, MSQUEUE_BUFFER);
    memset(poems[1], 0, MSQUEUE_BUFFER);

    int error_read = read(pipe_io, poems, 2 * MSQUEUE_BUFFER);
    poems[0][strlen(poems[0])] = '\0';
    poems[1][strlen(poems[1])] = '\0';

    if (error_read < 0)
    {
        perror("reading string 1 failed");
    }

    puts("[CHILD] Received poems:");

    for (int i = 0; i < 2; i++)
    {
        printf("[%d] %s\n", i + 1, poems[i]);
    }

    int random = random_generator(2, false);
    puts("[CHILD] Poem has been chosen");
    int status;
    MessageQueue msqueue;

    puts(poems[random]);
    puts("Szabad-e locsolni?");
    msqueue.mtype = MSQUEUE_TYPE;
    strncpy(msqueue.mtext, poems[random], MSQUEUE_BUFFER);
    status = msgsnd(msqueue_id, &msqueue, strlen(poems[random]) + 1, 0);

    if (status < 0)
    {
        perror("Message queue: sending message failed :(");
    }

    puts("[CHILD] Poem has been sent");
    exit(EXIT_SUCCESS);
}

static void application_command_save(Application* const application)
{
    if (application->is_edited)
    {
        application->file = fopen(FILENAME, "w");

        for (size_t i = 0; i < vector_get_size(application->vector); i++)
        {
            fprintf(application->file, "%s\n", vector_get_at(application->vector, i));
        }

        fclose(application->file);
        application->file = NULL;
        application->is_edited = false;
        puts("File has been saved successfully.");
    }
    else
    {
        puts("No edits have been performed. Saving skipped.");
    }
}

static void application_command_quit(Application* application)
{
    if (application->is_edited)
    {
        printf("The database has been edited.\nDo you want to save changes? [Y/N] > ");
        String* line = string_read_line(stdin);
        string_transform_to_upper(line);

        while (!string_are_equal_c(line, "Y") && !string_are_equal_c(line, "N") && !feof(stdin))
        {
            string_destroy(line);
            printf("Invalid input. Try again. [Y/N] > ");
            line = string_read_line(stdin);
            string_transform_to_upper(line);
        }

        if (string_are_equal_c(line, "Y"))
        {
            application_command_save(application);
        }

        string_destroy(line);
    }

    application->quit_state = true;
}

static void application_command_help(Application* application)
{
    puts("=== Easter Bunny's Poems ===");
    puts("Commands:");
    puts("\ti - insert; inserts a new poem.");
    puts("\tl - list; enumerates each poem in the database.");
    puts("\tw - sprinkle; throw water onto a girl according to the ancient Hungarian Easter-related folk tradition.");
    puts("\th - help; prints out all available commands.");
    puts("\ts - save; saves database.");
    puts("\tq - quit; quits the program if no edits were performed.");
    puts("\t          Otherwise, asks the user about saving the changes.");
    puts("\te [number] - edit; edits the poem at the specified index.");
    puts("\tr [number] - remove; removes the poem at the specified index.");
    puts("Remarks:");
    puts("\t- All commands can be capitalised.");
    puts("\t- Arguments must be separated by a whitespace character.");
    printf("\t- Indices must fall in the range of [1..'n'] (where 'n' == %lu).\n", vector_get_size(application->vector));
}

static void application_command_remove(Application* application, Argument argument)
{
    if (argument == NO_ARGUMENTS || argument > vector_get_size(application->vector))
    {
        fprintf(stderr,
                "Error: invalid index (%lu) - indices must fall in the range of [1..%lu].\n",
                argument, vector_get_size(application->vector));
    }
    else
    {
        vector_remove_at(application->vector, argument - 1);

        if (!application->is_edited)
        {
            application->is_edited = true;
        }
    }
}

static void application_command_edit(Application* application, Argument argument)
{
    if (argument == NO_ARGUMENTS || argument > vector_get_size(application->vector))
    {
        fprintf(stderr,
                "Error: invalid index (%lu) - indices must fall in the range of [1..%lu].\n",
                argument, vector_get_size(application->vector));
    }
    else
    {
        printf("Edit poem %lu > ", argument);
        String *edited_poem = string_read_line(stdin);

        while (string_are_equal_c(edited_poem, "") && !feof(stdin))
        {
            fprintf(stderr, "Invalid input: poem cannot be empty. Try again. > ");
            string_destroy(edited_poem);
            edited_poem = string_read_line(stdin);
        }

        vector_set_at(application->vector, argument - 1, edited_poem);

        if (!application->is_edited)
        {
            application->is_edited = true;
        }
    }
}

static Vector* application_tokenise_input(const String* const string)
{
    Vector* tokens = vector_construct();
    char* copy = ALLOCATE_ARRAY(char, string_get_size(string));
    strncpy(copy, string_get_data(string), string_get_size(string));
    char delimiters[] = " \t";
    char* token = strtok(copy, delimiters);

    while (token != NULL)
    {
        vector_append(tokens, string_construct(token));
        token = strtok(NULL, delimiters);
    }

    free(copy);
    return tokens;
}

static ApplicationCommand application_process_tokens(const Vector* const tokens)
{
    if (tokens == NULL)
    {
        // in case that vector 'tokens' is empty
        return (ApplicationCommand){NO_COMMAND, NO_ARGUMENTS};
    }

    size_t index = 0;
    bool continue_args = false;
    ApplicationCommand cmd = {NO_COMMAND, NO_ARGUMENTS};

    if (index == 0 && index < vector_get_size(tokens))
    {
        if (string_get_length(vector_get_string_at(tokens, index)) != 1)
        {
            // unrecognised command (must be exactly 1 character long)
            return (ApplicationCommand){ERROR, NO_ARGUMENTS};
        }
        else
        {
            // checks string command -- comparison of the first character
            switch (vector_get_at(tokens, index)[0])
            {
            // commands requiring no arguments
            case 'i':
            case 'I':
                return (ApplicationCommand){INSERT, NO_ARGUMENTS};
            case 'l':
            case 'L':
                return (ApplicationCommand){LIST, NO_ARGUMENTS};
            case 'w':
            case 'W':
                return (ApplicationCommand){SPRINKLE, NO_ARGUMENTS};
            case 'h':
            case 'H':
                return (ApplicationCommand){HELP, NO_ARGUMENTS};
            case 's':
            case 'S':
                return (ApplicationCommand){SAVE, NO_ARGUMENTS};
            case 'q':
            case 'Q':
                return (ApplicationCommand){QUIT, NO_ARGUMENTS};

            // commands requiring 1 argument
            case 'e':
            case 'E':
                cmd.command = EDIT;
                continue_args = true;
                break;
            case 'r':
            case 'R':
                cmd.command = REMOVE;
                continue_args = true;
                break;

            // unrecognised command
            default:
                return (ApplicationCommand){ERROR, NO_ARGUMENTS};
            }
        }
    }

    // if the command has an additional argument
    if (continue_args)
    {
        index++;

        if (index < vector_get_size(tokens))
        {
            char* end;
            // converts the second token to a 'long'
            // 0 is equivalend to NO_ARGUMENTS
            cmd.argument = strtoul(vector_get_at(tokens, index), &end, 10);
        }
        else
        {
            fprintf(stderr, "Error: missing argument.\n");
            // cmd.command = ERROR;
        }
    }

    return cmd;
}

static void application_execute_command(Application* const application)
{
    switch (application->command_to_execute.command)
    {
    // 0-argument functions
    case INSERT:
        application_command_insert(application);
        break;
    case LIST:
        application_command_list(application);
        break;
    case SPRINKLE:
        application_command_sprinkle(application);
        break;
    case HELP:
        application_command_help(application);
        break;
    case SAVE:
        application_command_save(application);
        break;
    case QUIT:
        application_command_quit(application);
        break;
    // 1-argument functions
    case EDIT:
        application_command_edit(application, application->command_to_execute.argument);
        break;
    case REMOVE:
        application_command_remove(application, application->command_to_execute.argument);
        break;
    // error message
    case ERROR:
        fprintf(stderr, "Error: unrecognised command.\n");
        break;
    // case NO_COMMAND: return;
    default:
        return;
    }
}