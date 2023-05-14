#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  std::shared_ptr<Command> curr_cmd = SmallShell::getInstance().getCurrentCmd();

  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGSTOP);
    SmallShell::getInstance().setCurrentCmd(nullptr);
    SmallShell::getInstance().getJobs().addJob(curr_cmd, true);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was stopped" << endl;
  }
  cout << "smash: got ctrl-Z" << endl;
  return;
}

void ctrlCHandler(int sig_num)
{
  cout << "smash: got ctrl-C" << endl;
  std::shared_ptr<Command> curr_cmd = SmallShell::getInstance().getCurrentCmd();
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGINT);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was killed" << endl;
    SmallShell::getInstance().setCurrentCmd(nullptr);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was killed" << endl;
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
