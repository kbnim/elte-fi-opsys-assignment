#include <signal.h>

#include "hdr/String.h"
#include "hdr/Vector.h"
#include "hdr/Application.h"
#include "hdr/PosixUtils.h"

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