#ifndef PosixUtils_H
#define PosixUtils_H

/* ===== PIPE MACROS ===== */
/* Number of ports in an arbitrary POSIX pipe. */
#define IO_PORTS 2
/* Port for receiving or reading from pipe. */
#define RECEIVE 0
/* Port for sending or writing to pipe. */
#define SEND 1

/* ===== MESSAGE QUEUE ==== */
/* Message queue's default capacity of 'mtext'. */
#define MSQUEUE_BUFFER 1024
/* Message queue's 'mtype' value. */
#define MSQUEUE_TYPE 5

/* Structure for POSIX message queue. */
typedef struct MessageQueue {
    long mtype;
    char mtext[MSQUEUE_BUFFER];
} MessageQueue;

/* Structure for shared memory. */
typedef struct SharedMemory {
    // could be a matrix, but for now, this is simpler
    char poem01[MSQUEUE_BUFFER];
    char poem02[MSQUEUE_BUFFER];
} SharedMemory;

/*
  Generates a random integer.
  If 'closed_range' is true, the range of values is [1..max]. 
  Otherwise, it is [0..max).
*/
int random_generator(int max, bool closed_range);

/* Signal handler for signals coming from child process. */
void signal_handler_from_child(int signal_number);

#endif // PosixUtils_H