#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[])
{
    // std::cout << "start function\n";
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-Z handler!!!!!!!\n");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-C handler\n");
    }
    std::cout << "after signals";
    // TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance();
    // std::cout << "after creating smash\n";
    while (true)
    {
        // std::cout << "inside while\n";

        std::cout << smash.getPrompt() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}