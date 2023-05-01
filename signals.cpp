#include <iostream>
#include <signal.h>
#include "signals.h"
#include <unistd.h>
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  Command *curr_cmd = SmallShell::getInstance().getCurrentCmd();
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGSTOP);
    SmallShell::getInstance().setCurrentCmd(nullptr);
    SmallShell::getInstance().getJobs().addJob(curr_cmd, true);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was stopped" << endl;
  }
  cout << "smash: got ctrl-Z" << endl;
}

void ctrlCHandler(int sig_num)
{
  cout << "smash: got ctrl-C" << endl;
  Command *curr_cmd = SmallShell::getInstance().getCurrentCmd();
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGINT);
    SmallShell::getInstance().setCurrentCmd(nullptr);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was killed" << endl;
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
