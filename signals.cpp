#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
<<<<<<< Updated upstream
  std::shared_ptr<Command> curr_cmd = SmallShell::getInstance().getCurrentCmd();
=======
  Command *curr_cmd = SmallShell::getInstance().getCurrentCmd();
>>>>>>> Stashed changes
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGSTOP);
    SmallShell::getInstance().setCurrentCmd(nullptr);
    SmallShell::getInstance().getJobs().addJob(curr_cmd, true);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was stopped" << endl;
  }
  cout << "smash: got ctrl-Z" << endl;
<<<<<<< Updated upstream
  return;
=======
>>>>>>> Stashed changes
}

void ctrlCHandler(int sig_num)
{
  cout << "smash: got ctrl-C" << endl;
<<<<<<< Updated upstream
  std::shared_ptr<Command> curr_cmd = SmallShell::getInstance().getCurrentCmd();
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGINT);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was killed" << endl;
=======
  Command *curr_cmd = SmallShell::getInstance().getCurrentCmd();
  if (curr_cmd)
  {
    kill(curr_cmd->getPid(), SIGINT);
>>>>>>> Stashed changes
    SmallShell::getInstance().setCurrentCmd(nullptr);
    cout << "smash: process " << int(curr_cmd->getPid()) << " was killed" << endl;
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
