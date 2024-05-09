#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>

#include "PosixUtils.h"

int random_generator(int max_value, bool closed_range)
{
    return (rand() % max_value) + closed_range;
}

void signal_handler_from_child(int signal_number)
{
    (void)signal_number;
    puts("Signal from child process has arrived");
}