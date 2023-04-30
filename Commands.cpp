#include <unistd.h>
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

SmallShell::SmallShell() : jobs_list(), smash_name("smash")
{
  // TODO: add your implementation
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
  string last_arg = args.back();
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
    cout << "external commands";
    // if (inBackground)
    // {
    //   cmd += "&";
    // }
    // return new ExternalCommand(cmd_without_changes, cmd, args, inBackground);
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
  cout << "in JobsCommand command";
}
ForegroundCommand::ForegroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  cout << "in ForegroundCommand command";
}
BackgroundCommand::BackgroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  cout << "in BackgroundCommand command";
}
QuitCommand::QuitCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  cout << "in QuitCommand command";
}
KillCommand::KillCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  cout << "in KillCommand command";
}

/************** !!!!executes!!!! ******************/
void chpromptCommand::execute()
{
  SmallShell::getInstance().setPrompt(this->prompt);
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
}
void BackgroundCommand::execute()
{
}
void QuitCommand::execute()
{
}
void KillCommand::execute()
{
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
}
