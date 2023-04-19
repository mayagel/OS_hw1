#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  cout << "smash: got ctrl-C" << endl;
}

void ctrlCHandler(int sig_num)
{
  cout << "smash: got ctrl-C" << endl;
  pid_t pid;
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
