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

#define MAX_NUMBER_OF_CHILDREN 4

#define DEFAULT_BUFFER_SIZE 32

#define ALLOCATE(TYPE) (TYPE *)calloc(1, sizeof(TYPE))
#define ALLOCATE_ARRAY(TYPE, size) (TYPE *)calloc(size, sizeof(TYPE))
#define DOUBLE_ARRAY(array, capacity, TYPE) (TYPE *)realloc((array), (capacity) * (2) * sizeof(TYPE))

#define IO_PORTS 2
#define RECEIVE 0
#define SEND 1

#define SEMOP_UP 1
#define SEMOP_DOWN -1

/* ===== MESSAGE QUEUE ==== */

#define MSQUEUE_BUFFER 1024
#define MSQUEUE_TYPE 5

typedef struct MessageQueue
{
    long mtype;
    char mtext[MSQUEUE_BUFFER];
} MessageQueue;

char program_name[MSQUEUE_BUFFER];

typedef struct SharedMemory
{
    // could be a matrix, but for now, this is simpler
    char poem01[MSQUEUE_BUFFER];
    char poem02[MSQUEUE_BUFFER];
} SharedMemory;

/* ===== STRING API ==== */

typedef struct String
{
    char *data;
    size_t length;
    size_t size;
    bool is_used;
} String;

String *string_construct(const char *const str);
void string_destroy(String *str);
int string_compare(const String *const left, const String *const right);
bool string_are_equal(const String *const left, const String *const right);
bool string_are_equal_c(const String *const left, const char *const right);
void string_transform_to_upper(String *const string);
String *string_read_line(FILE *source);

/* ===== VECTOR API ===== */

typedef struct Vector
{
    String **data;
    size_t size;
    size_t capacity;
    size_t used_count;
} Vector;

Vector *vector_construct();
void vector_destroy(Vector *vector);
void vector_print(const Vector *const vector);
void vector_append(Vector *vector, String *const string);
void vector_double_capacity(Vector *vector);
char *vector_get_at(const Vector *const vector, size_t index);
void vector_set_at(Vector *vector, size_t index, String *const string);
void vector_remove_at(Vector *vector, size_t index);
void vector_set_used(Vector *vector, size_t index);

/* ===== MENU API ===== */

#define FILENAME "./poems.txt"

typedef enum Command
{
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

typedef struct ApplicationCommand
{
    Command command;
    Argument argument;
} ApplicationCommand;

typedef struct Application
{
    FILE *file;
    Vector *vector;
    bool quit_state;
    bool is_edited;
    ApplicationCommand command_to_execute;
} Application;

void application_initialise(Application *const application);
int application_run(Application *application);

void application_command_insert(Application *application);
void application_command_list(Application *application);
void application_command_sprinkle(Application *application);
pid_t application_command_sprinkle_child_async(int, int);
void application_command_help(Application *application);
void application_command_save(Application *application);
void application_command_quit(Application *application);
void application_command_remove(Application *application, Argument argument);
void application_command_edit(Application *application, Argument argument);

Vector *application_tokenise_input(const String *const string);
ApplicationCommand application_process_tokens(const Vector *const tokens);
void application_execute_command(Application *application);

int random_generator(int, bool);
// void signal_handler_quit(int signal_number);
// void signal_handler_interrupt(int signal_number);
void signal_handler_from_child(int signal_number);

// semaphores
int semaphore_create(const char *pathname, int semaphore_value);
void semaphore_operation(int semaphore_id, int operation);
void semaphore_delete(int semaphore_id);

int main(int argc, char **argv)
{
    signal(SIGUSR1, signal_handler_from_child);

    // to avoid compiler warnings
    (void)argc;

    // setting up global variable for message queue
    strncpy(program_name, argv[0], MSQUEUE_BUFFER);
    program_name[MSQUEUE_BUFFER - 1] = '\0';

    // actual program initialisation and execution
    Application app;
    application_initialise(&app);
    return application_run(&app);
}

/* ===== STRING IMPLEMENTATION ===== */

String *string_construct(const char *const str)
{
    if (str == NULL)
    {
        return string_construct("");
    }

    String *string = ALLOCATE(String);

    if (string == NULL)
    {
        return NULL;
    }

    string->length = strlen(str);
    string->size = string->length + 1;
    string->data = ALLOCATE_ARRAY(char, string->size);
    string->is_used = false;
    strncpy(string->data, str, string->size);
    return string;
}

void string_destroy(String *str)
{
    if (str != NULL)
    {
        free(str->data);
        free(str);
    }

    str = NULL;
}

int string_compare(const String *const left, const String *const right)
{
    return strcmp(left->data, right->data);
}

String *string_read_line(FILE *source)
{
    char *buffer = ALLOCATE_ARRAY(char, DEFAULT_BUFFER_SIZE);
    size_t size = 0;
    size_t capacity = DEFAULT_BUFFER_SIZE;
    int character = fgetc(source);

    while (character != '\n' && character != EOF)
    {
        if (size == capacity)
        {
            buffer = DOUBLE_ARRAY(buffer, capacity, char);
            capacity *= 2;
        }

        buffer[size] = (char)character;
        size++;

        character = fgetc(source);
    }

    if (size == 0 && character == EOF)
    {
        free(buffer);
        return string_construct("");
    }

    buffer[size] = '\0';
    char *trimmed_buffer = ALLOCATE_ARRAY(char, size + 1);
    strncpy(trimmed_buffer, buffer, size + 1);
    trimmed_buffer[size] = '\0';

    String *string = string_construct(trimmed_buffer);
    free(buffer);
    free(trimmed_buffer);
    return string;
}

bool string_are_equal(const String *const left, const String *const right)
{
    return string_compare(left, right) == 0;
}

bool string_are_equal_c(const String *const left, const char *const right)
{
    return strncmp(left->data, right, left->size) == 0;
}

void string_transform_to_upper(String *const string)
{
    for (size_t i = 0; i < string->length; i++)
    {
        if (isalpha(string->data[i]))
        {
            string->data[i] = toupper(string->data[i]);
        }
    }
}

/* ===== VECTOR IMPLEMENTATION ===== */

Vector *vector_construct()
{
    Vector *vector = ALLOCATE(Vector);

    if (vector == NULL)
    {
        return NULL;
    }

    vector->data = ALLOCATE_ARRAY(String *, 1);

    if (vector->data == NULL)
    {
        return NULL;
    }

    vector->capacity = 1;
    vector->size = 0;
    vector->used_count = 0;
    return vector;
}

void vector_destroy(Vector *vector)
{
    if (vector != NULL)
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        free(vector);
    }

    vector = NULL;
}

void vector_print(const Vector *const vector)
{
    if (vector->size == 0)
    {
        puts("(empty)");
    }
    else
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            printf("[%lu] %s", i + 1, vector_get_at(vector, i));

            if (vector->data[i]->is_used)
            {
                printf(" (USED)");
            }

            printf("\n");
        }
    }
}

void vector_append(Vector *vector, String *const string)
{
    if (vector == NULL)
        return;

    if (vector->capacity == vector->size)
    {
        vector_double_capacity(vector);
    }

    vector->data[vector->size] = string;
    vector->size++;
}

void vector_double_capacity(Vector *vector)
{
    vector->data = DOUBLE_ARRAY(vector->data, vector->capacity, String *);
    vector->capacity = 2 * vector->capacity;

    for (size_t i = vector->size; i < vector->capacity; i++)
    {
        vector->data[i] = NULL;
    }
}

char *vector_get_at(const Vector *const vector, size_t index)
{
    if (index >= vector->size)
    {
        return NULL;
    }

    return vector->data[index]->data;
}

void vector_set_at(Vector *vector, size_t index, String *const string)
{
    if (index >= vector->size)
    {
        return;
    }

    string_destroy(vector->data[index]);
    vector->data[index] = string;
}

/*
  previous trial of the function
*/

/*
void vector_remove_at(Vector* vector, size_t index)
{
    if (index >= vector->size)
    {
        return;
    }

    string_destroy(vector->data[index]);

    for (size_t i = index; i < vector->size - 1; i++)
    {
        vector->data[i] = vector->data[i + 1];
    }

    if (vector->data[vector->size] != NULL)
    {
        string_destroy(vector->data[vector->size]);
    }

    vector->size--;
}
*/

/*
  Note: I'm absolutely sure there's a far more efficient algorithm
  that does not require to deep-copy or needlessly allocate memory
  on the heap. The primary reason of choosing this solution was that
  it proved to be the most memory-safe and avoided memory leaks.
*/
void vector_remove_at(Vector *vector, size_t index)
{
    if (index >= vector->size)
    {
        return;
    }

    // if the removal does not cause the vector to become empty
    if (vector->size - 1 != 0)
    {
        bool str_is_used = vector->data[index]->is_used;
        size_t vec_used_count = vector->used_count;
        String **string_arr = ALLOCATE_ARRAY(String *, vector->size - 1);
        size_t arr_index = 0;

        // deep-copying strings that are supposed to be preserved
        for (size_t i = 0; i < vector->size; i++)
        {
            if (i != index)
            {
                string_arr[arr_index] = string_construct(vector->data[i]->data);
                string_arr[arr_index]->is_used = vector->data[i]->is_used;
                arr_index++;
            }
        }

        // deallocating original array
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        vector->data = string_arr;
        vector->size = arr_index;
        vector->capacity = arr_index;
        vector->used_count = str_is_used ? vec_used_count - 1 : vec_used_count;
    }
    else // avoiding calloc()-ing memory of 0 bytes
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        vector->data = ALLOCATE_ARRAY(String *, 1);
        vector->size = 0;
        vector->capacity = 1;
    }
}

void vector_set_used(Vector *vector, size_t index)
{
    if (vector->data[index]->is_used == false)
    {
        vector->data[index]->is_used = true;
        vector->used_count++;
    }
}

/* ===== MENU API ===== */

void application_initialise(Application *const application)
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

int application_run(Application *application)
{
    puts("=== Easter Bunny's Poems ===");
    vector_print(application->vector);

    while (!application->quit_state && !feof(stdin))
    {
        printf("> ");
        String *input = string_read_line(stdin);
        Vector *tokens = application_tokenise_input(input);
        application->command_to_execute = application_process_tokens(tokens);
        application_execute_command(application);
        vector_destroy(tokens);
        string_destroy(input);
    }

    vector_destroy(application->vector);
    return application->file != NULL ? fclose(application->file) : EXIT_SUCCESS;
}

void application_command_insert(Application *application)
{
    printf("Insert new poem > ");
    String *poem = string_read_line(stdin);

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

void application_command_list(Application *application)
{
    vector_print(application->vector);
}

void application_command_sprinkle(Application *application)
{
    // minimal error handling
    if (application->vector->size < 2)
    {
        fprintf(stderr, "Error: not enough poems to select from.\n");
        return;
    }

    if (application->vector->used_count > application->vector->size - 2)
    {
        fprintf(stderr, "Error: you need at least 2 unused poems in the database.\n");
        return;
    }

    srand(time(NULL));
    int random = random_generator(MAX_NUMBER_OF_CHILDREN, true);

    int child_status;
    pid_t child_process;

    int poem_index_01 = random_generator(application->vector->size, false);

    while (application->vector->data[poem_index_01]->is_used == true)
    {
        poem_index_01 = random_generator(application->vector->size, false);
    }

    int poem_index_02 = random_generator(application->vector->size, false);

    // so that the two indices are different
    while (application->vector->data[poem_index_02]->is_used == true ||
           poem_index_02 == poem_index_01)
    {
        poem_index_02 = random_generator(application->vector->size, false);
    }

    size_t poem_01_size = application->vector->data[poem_index_01]->size;
    size_t poem_02_size = application->vector->data[poem_index_02]->size;

    MessageQueue msqueue;
    int msqueue_id;
    int msqueue_status;
    key_t key = ftok(program_name, 1);

    char poems[2][MSQUEUE_BUFFER];
    memset(poems[0], 0, MSQUEUE_BUFFER);
    memset(poems[1], 0, MSQUEUE_BUFFER);

    strncpy(poems[0], application->vector->data[poem_index_01]->data, poem_01_size);
    strncpy(poems[1], application->vector->data[poem_index_02]->data, poem_02_size);
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
        string_are_equal_c(application->vector->data[poem_index_01], msqueue.mtext) ? poem_index_01 : poem_index_02;

    vector_set_used(application->vector, used_index);
}

pid_t application_command_sprinkle_child_async(int pipe_io, int msqueue_id)
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

void application_command_save(Application *const application)
{
    if (application->is_edited)
    {
        application->file = fopen(FILENAME, "w");

        for (size_t i = 0; i < application->vector->size; i++)
        {
            fprintf(application->file, "%s\n", application->vector->data[i]->data);
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

void application_command_quit(Application *application)
{
    if (application->is_edited)
    {
        printf("The database has been edited.\nDo you want to save changes? [Y/N] > ");
        String *line = string_read_line(stdin);
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

void application_command_help(Application *application)
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
    printf("\t- Indices must fall in the range of [1..'n'] (where 'n' == %lu).\n", application->vector->size);
}

void application_command_remove(Application *application, Argument argument)
{
    if (argument == NO_ARGUMENTS || argument > application->vector->size)
    {
        fprintf(stderr,
                "Error: invalid index (%lu) - indices must fall in the range of [1..%lu].\n",
                argument, application->vector->size);
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

void application_command_edit(Application *application, Argument argument)
{
    if (argument == NO_ARGUMENTS || argument > application->vector->size)
    {
        fprintf(stderr,
                "Error: invalid index (%lu) - indices must fall in the range of [1..%lu].\n",
                argument, application->vector->size);
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

Vector *application_tokenise_input(const String *const string)
{
    Vector *tokens = vector_construct();
    char *copy = ALLOCATE_ARRAY(char, string->size);
    strncpy(copy, string->data, string->size);
    char delimiters[] = " \t";
    char *token = strtok(copy, delimiters);

    while (token != NULL)
    {
        vector_append(tokens, string_construct(token));
        token = strtok(NULL, delimiters);
    }

    free(copy);
    return tokens;
}

ApplicationCommand application_process_tokens(const Vector *const tokens)
{
    if (tokens == NULL)
    {
        // in case that vector 'tokens' is empty
        return (ApplicationCommand){NO_COMMAND, NO_ARGUMENTS};
    }

    size_t index = 0;
    bool continue_args = false;
    ApplicationCommand cmd = {NO_COMMAND, NO_ARGUMENTS};

    if (index == 0 && index < tokens->size)
    {
        if (tokens->data[index]->length != 1)
        {
            // unrecognised command (must be exactly 1 character long)
            return (ApplicationCommand){ERROR, NO_ARGUMENTS};
        }
        else
        {
            // checks string command -- comparison of the first character
            switch (tokens->data[index]->data[0])
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

        if (index < tokens->size)
        {
            char *end;
            // converts the second token to a 'long'
            // 0 is equivalend to NO_ARGUMENTS
            cmd.argument = strtoul(tokens->data[index]->data, &end, 10);
        }
        else
        {
            fprintf(stderr, "Error: missing argument.\n");
            // cmd.command = ERROR;
        }
    }

    return cmd;
}

void application_execute_command(Application *const application)
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

int random_generator(int max_value, bool closed_range)
{
    return (rand() % max_value) + closed_range;
}

/*
 * Though, I ended up not using semaphores,
 * I decides to keep these functions in the source file
 * just in case ;)
 */

int semaphore_create(const char *pathname, int semaphore_value)
{
    key_t semaphore_key = ftok(pathname, 1);
    int semaphore_id = semget(semaphore_key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

    if (semaphore_id < 1)
    {
        perror("Creating semaphore failed");
    }

    if (semctl(semaphore_id, 0, SETVAL, semaphore_value) < 0)
    {
        perror("semctl failed");
    }

    return semaphore_id;
}

void semaphore_operation(int semaphore_id, int operation)
{
    struct sembuf semaphore_op;
    semaphore_op.sem_num = 0;
    semaphore_op.sem_op = operation;
    semaphore_op.sem_flg = 0;

    if (semop(semaphore_id, &semaphore_op, 1) < 0)
    {
        perror("semop failed");
    }
}

void semaphore_delete(int semaphore_id)
{
    semctl(semaphore_id, 0, IPC_RMID);
}

void signal_handler_from_child(int signal_number)
{
    (void)signal_number;
    puts("Signal from child process has arrived");
}