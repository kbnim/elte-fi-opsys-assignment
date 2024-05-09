#include <signal.h>

#include "src/String.h"
#include "src/Vector.h"
#include "src/Application.h"
#include "src/PosixUtils.h"

int main(int argc, char **argv)
{
    signal(SIGUSR1, signal_handler_from_child);

    // to avoid compiler warnings
    (void)argc;

    // actual program initialisation and execution
    Application app;
    application_initialise(&app, argv[0]);
    return application_run(&app);
}