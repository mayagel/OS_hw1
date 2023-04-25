#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[])
{
    std::cout << "start function";
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-C handler");
    }
    std::cout << "after signals";
    // TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance();
    std::cout << "after creating smash";
    while (true)
    {
        std::cout << "inside while";

        std::cout << smash.getPrompt() << "> ";
        std::cout << "after printing 'smash >'";

        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}