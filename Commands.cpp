#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

/************** !!!!helpers!!!! ******************/

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) // use this to delete whitespaces from command
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) // slpit the commandline to list of strs
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

int _parseCommandLine(string cmd_line, vector<string> &args) // slpit the commandline to list of strs
{
  FUNC_ENTRY()
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args.push_back(s);
  }
  return args.size();

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

bool _isBackgroundComamnd(string cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

void _removeBackgroundSign(string &cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

/************** !!!!SmallShell implements!!!! ******************/

SmallShell::SmallShell() : smash_name("smash"), jobs_list()
{
  // cout << "smash: " << smash_name << "> " << endl;
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  if (!cmd_line)
    return nullptr;
  string org_cmd = string(cmd_line);
  vector<string> args;
  if (!_parseCommandLine(org_cmd, args))
  {
    return nullptr;
  }
  /*string last_arg = args.back();
  args.erase(args.end() - 1);
  if (_isBackgroundComamnd(last_arg))
  {
    // cout << "found &!!!\n";
    last_arg.pop_back();
    // cout << "delete &\n\n\n";
    // cout << "now last arg is: " << string(last_arg) << endl;
  }
  args.push_back(last_arg);
  // args.push_back(last_arg);
  //  string fix_cmd = org_cmd;
  //  _removeBackgroundSign(org_cmd); // maybe delete whitespace at the end
*/
  if (_isBackgroundComamnd(args.back()))
  {
    args.back().pop_back();
    if (args.back().empty())
    {
      args.pop_back();
    }
  }
  if (args[0] == "chprompt")
  {
    return new chpromptCommand(org_cmd, args);
  }
  else if (args[0] == "showpid")
  {
    return new ShowPidCommand(org_cmd, args);
  }
  else if (args[0] == "pwd")
  {
    return new GetCurrDirCommand(org_cmd, args);
  }
  else if (args[0] == "cd")
  {
    return new ChangeDirCommand(org_cmd, args);
  }
  else if (args[0] == "jobs")
  {
    return new JobsCommand(org_cmd, args);
  }
  else if (args[0] == "fg")
  {
    return new ForegroundCommand(org_cmd, args);
  }
  else if (args[0] == "bg")
  {
    return new BackgroundCommand(org_cmd, args);
  }
  else if (args[0] == "quit")
  {
    return new QuitCommand(org_cmd, args);
  }
  else if (args[0] == "kill")
  {
    return new KillCommand(org_cmd, args);
  }
  // else if (args[0] == "setcore")
  // {
  //   return new SetcoreCommand(org_cmd, args);
  // }
  else
  {
    // cout << "external commands" << endl;
    return new ExternalCommand(org_cmd, args);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{

  // TODO: Add your implementation here
  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::printJobs()
{
  // cout << "in print jobs" << endl;
  jobs_list.printJobsList();
}

void SmallShell::removeJobById(int jobId)
{
  // cout << "in remove job by id" << endl;
  jobs_list.removeJobById(jobId);
}

/************** !!!!constructors!!!! ******************/
GetCurrDirCommand::GetCurrDirCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  // if (!getcwd())
  // {
  //   return nullptr;
  // }
  pwd = getcwd(NULL, 0);
  // cout << getcwd(NULL, 0) << endl;
}
ChangeDirCommand::ChangeDirCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  chdir(args[1].c_str()); // maybe need pay attention to leak
  // handeling error at the ends!
}
JobsCommand::JobsCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  // cout << "in JobsCommand command" << endl;
}
ForegroundCommand::ForegroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  // cout << "in ForegroundCommand command" << endl;
  if (args.size() == 1)
  {
    // cout << "args size is 1" << endl;
    int res;
    SmallShell::getInstance().getJobs().getLastStoppedJob(&res);

    // cout << "res is: " << res << endl;
    job_to_fg = res;
  }
  else if (args.size() == 2)
  {
    // cout << "args size is 2" << endl;
    // check if valid
    job_to_fg = stoi(args[1]);
  }
}
BackgroundCommand::BackgroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  // cout << "in BackgroundCommand command" << endl;
  if (args.size() == 1)
  {
    // cout << "args size is 1" << endl;
    int res;
    SmallShell::getInstance().getJobs().getLastStoppedJob(&res);

    // cout << "res is: " << res << endl;
    job_to_bg = res;
  }
  else if (args.size() == 2)
  {
    // cout << "args size is 2" << endl;
    // check if valid
    job_to_bg = stoi(args[1]);
  }

  // cout << "after BackgroundCommand command" << endl;
}
QuitCommand::QuitCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  cout << "in QuitCommand command" << endl;
}
KillCommand::KillCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  job_to_kill = stoi(args[2]);
  // cout << "job to kill is: " << job_to_kill << endl;
  // cout << "signal to send is: " << args[1] << endl;
  signal_num = (-1) * stoi(args[1]);
  cout << "in KillCommand command" << endl;
}

ExternalCommand::ExternalCommand(string cmd_line, vector<string> args, pid_t pid) : Command(cmd_line, args, pid)
{
  cout << "job id is: " << job_id << endl;
  cout << "in ExternalCommand command" << endl;
}

/************** !!!!executes!!!! ******************/
void chpromptCommand::execute()
{
  // cout << this->prompt << endl;
  // cout << SmallShell::getInstance().getPrompt() << endl;
  SmallShell::getInstance().setPrompt(this->prompt);
  // cout << SmallShell::getInstance().getPrompt() << endl;
}
void ShowPidCommand::execute()
{

  cout << "smash pid is " << SmallShell::getInstance().getPidSmash() << endl;
  // SmallShell::getInstance().setPrompt(this->prompt);
}
void GetCurrDirCommand::execute()
{
  cout << pwd << endl;
  // SmallShell::getInstance().setPrompt(this->prompt);
}
void ChangeDirCommand::execute()
{
}
void JobsCommand::execute()
{
  SmallShell::getInstance().printJobs();
}
void ForegroundCommand::execute()
{
  Command *cmd = SmallShell::getInstance().getJobs().getJobById(job_to_fg)->getCmd();
  JobsList::JobEntry *the_job = SmallShell::getInstance().getJobs().getJobById(job_to_fg);
  if (the_job->isStopped())
  {
    // the_job->setStopped(false);
    if (kill(the_job->getPid(), SIGCONT) == -1)
    {
      perror("smash error: kill failed");
    }
  }
  cout << the_job->getCommand() << " : " << the_job->getPid() << endl;
  SmallShell::getInstance().setCurrentCmd(cmd);
  SmallShell::getInstance().removeJobById(job_to_fg);
  if (waitpid(cmd->getPid(), nullptr, WUNTRACED) == -1)
  {
    perror("smash error: waitpid failed");
  }
  // SmallShell::getInstance().setCurrentCmdPid(-1);
  SmallShell::getInstance().setCurrentCmd(nullptr);
  // SmallShell::getInstance().setCurrentJobId(-1);
}
void BackgroundCommand::execute()
{
  cout << "in BackgroundCommand execute" << endl;
  cout << "job id is: " << job_to_bg << endl;
  cout << SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getCommand() << " : " << SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getPid() << endl;
  SmallShell::getInstance().getJobs().getJobById(job_to_bg)->setStopped(false);
  cout << "in BackgroundCommand execute" << endl;
  if (kill(SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getPid(), SIGCONT) == -1)
  {
    cout << "smash error: kill failed" << endl;
  }
}
void QuitCommand::execute()
{
}
void KillCommand::execute()
{
  pid_t pid_to_kill = SmallShell::getInstance().getJobs().getJobById(job_to_kill)->getPid();
  JobsList::JobEntry *job = SmallShell::getInstance().getJobs().getJobById(job_to_kill);
  if (SIGCONT == signal_num)
  {
    if (job->isStopped())
    {
      job->setStopped(false);
    }
  }
  if (signal_num == SIGSTOP)
  {
    if (!job->isStopped())
    {
      job->setStopped(true);
    }
  }
  if (kill(pid_to_kill, signal_num) == -1)
  {
    perror("smash error: kill failed");
  }
  else
  {
    cout << "signal number " << signal_num << " was sent to pid " << pid_to_kill << endl;
  }
}

void ExternalCommand::execute()
{
  pid_t temp_pid = fork();
  setpgrp();
  if (temp_pid == 0) // son
  {
    if (cmd_str.find("*") != string::npos || cmd_str.find("?") != string::npos)
    {
      // string bash_cmd = "/bin/bash";
      if (execl("/bin/bash", "/bin/bash", "-c", cmd_str.c_str(), NULL) == -1)
      {
        perror("smash error: execl failed");
        exit(1);
      }
    }
    else
    {
      vector<char *> char_args;
      for (auto &arg : args)
      {
        char_args.push_back((char *)arg.c_str());
      }
      char_args.push_back(NULL);
      if (execvp(args[0].c_str(), char_args.data()) == -1)
      {
        perror("smash error: execl failed");
        exit(1);
      }
    }
  }
  else // father
  {
    this->pid = temp_pid;
    if (_isBackgroundComamnd(cmd_str))
    {
      // cout << "add background command to list" << endl;
      cout << "pid is: " << temp_pid << endl;
      cout << "cmd is: " << cmd_str << endl;
      cout << "job id is: " << this->getJobId() << endl;
      SmallShell::getInstance().getJobs().addJob(this, false);
    }
    else
    {
      SmallShell::getInstance().setCurrentCmd(this);
      waitpid(temp_pid, NULL, WUNTRACED);
      SmallShell::getInstance().setCurrentCmd();
    }
  }
}
/************** !!!!other-implements!!!! ******************/

void JobsList::removeFinishedJobs()
{
  for (auto &[key, job] : jbs_map)
  {

    auto wait_stat = waitpid(job.getPid(), NULL, WNOHANG);
    if (wait_stat == -1)
    {
      perror("smash error: waitpid failed");
    }
    else if (wait_stat != 0)
    {
      jbs_map.erase(key);
    }
  }
}

void JobsList::printJobsList()
{
  removeFinishedJobs();
  for (auto &[key, job] : jbs_map)
  {
    time_t now = time(NULL);
    // int seconds = difftime(now, job.getStartTime());
    cout << "[" << key << "] " << job.getCommand() << " : " << job.getPid() << " " << int(difftime(now, job.getStartTime())) << " secs";
    if (job.isStopped())
    {
      cout << " (stopped)";
    }
    cout << endl;
  }
}

JobsList::JobsList() : jbs_map()
{

  // cout << SmallShell::getInstance().getPrompt() << endl;
  // cout << "in JobsList constructor" << endl;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
  JobEntry *res = nullptr;
  for (auto &[key, job] : jbs_map)
  {
    if (job.isStopped())
    {
      *jobId = key;
      res = &job;
    }
  }
  return res;
}

void JobsList::addJob(Command *cmd, bool isStopped)
{
  removeFinishedJobs();
  if (cmd->getJobId() != -1)
  {
    jbs_map.insert({cmd->getJobId(), JobEntry(cmd->getJobId(), isStopped, cmd)});
    return;
  }
  int max_id = 0;
  for (auto &[key, job] : jbs_map)
  {
    if (key > max_id)
    {
      max_id = key;
    }
  }
  // JobEntry job(max_id, isStopped, cmd);
  cout << "max id is: " << max_id << endl;
  jbs_map.insert({max_id + 1, JobEntry(max_id, isStopped, cmd)});
  cout << "map size is: " << jbs_map.size() << endl;
  // JobEntry job(cmd, isStopped);
  // jbs_map.insert({job.getPid(), job});
}

JobsList::JobEntry::JobEntry(int id, bool isStopped, Command *cmd) : job_id(id), is_stopped(isStopped), cmd(cmd)
{
  // this->pid = cmd->getPid();
  this->start_time = time(NULL);
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  return &jbs_map.at(jobId);
}

void JobsList::removeJobById(int jobId)
{
  jbs_map.erase(jobId);
}