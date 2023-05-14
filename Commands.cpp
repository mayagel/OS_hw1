#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <sys/wait.h>
#include <fcntl.h>
#include <iomanip>
#include "Commands.h"
#include <signal.h>
#include <fstream>
#include <thread>
#include <sched.h>
#include <sys/stat.h>
#include <csignal>

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

enum CMD_TYPE
{
  REGULAR,
  REDIRECTION,
  REDIRECTION_APPEND,
  PIPE,
  PIPE_ERR,
};

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

SmallShell::SmallShell() : curr_cmd(nullptr), smash_name("smash"), jobs_list(), last_wd("")
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

CMD_TYPE commandType(string line, int &index)
{
  if (line.find(">") != string::npos)
  {
    index = line.find(">"); // index of the first >
    if (line.find(">>") != string::npos)
    {
      return REDIRECTION_APPEND;
    }
    return REDIRECTION;
  }
  else if (line.find("|") != string::npos)
  {
    index = line.find("|"); // index of the first |
    if (line.find("|&") != string::npos)
    {
      return PIPE_ERR;
    }
    return PIPE;
  }
  return REGULAR;
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
std::shared_ptr<Command> SmallShell::CreateCommand(const char *cmd_line)
{
  if (!cmd_line || strcmp(cmd_line, "") == 0)
    return nullptr;
  string org_cmd = string(cmd_line);
  int index = 0;
  CMD_TYPE cmd_type = commandType(org_cmd, index);
  vector<string> args;
  if (cmd_type == REDIRECTION || cmd_type == REDIRECTION_APPEND)
  {
    args.push_back(_trim(string(org_cmd.substr(0, index))));

    args.push_back(_trim(string(org_cmd.substr(index + 1 + (cmd_type == REDIRECTION_APPEND), org_cmd.length()))));
    return std::make_shared<RedirectionCommand>(RedirectionCommand(org_cmd, cmd_type == REDIRECTION_APPEND, args));
  }

  if (cmd_type == PIPE || cmd_type == PIPE_ERR)
  {
    args.push_back(_trim(string(org_cmd.substr(0, index))));
    args.push_back(_trim(string(org_cmd.substr(index + 1 + (cmd_type == PIPE_ERR), org_cmd.length()))));
    return std::make_shared<PipeCommand>(PipeCommand(org_cmd, cmd_type == PIPE_ERR, args));
  }

  if (!_parseCommandLine(org_cmd, args))
  {
    return nullptr;
  }
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
    return std::make_shared<chpromptCommand>(chpromptCommand(org_cmd, args));
  }
  else if (args[0] == "showpid")
  {
    return std::make_shared<ShowPidCommand>(ShowPidCommand(org_cmd, args));
  }
  else if (args[0] == "pwd")
  {
    return std::make_shared<GetCurrDirCommand>(GetCurrDirCommand(org_cmd, args));
  }
  else if (args[0] == "cd")
  {
    return std::make_shared<ChangeDirCommand>(ChangeDirCommand(org_cmd, args));
  }
  else if (args[0] == "jobs")
  {
    return std::make_shared<JobsCommand>(JobsCommand(org_cmd, args));
  }
  else if (args[0] == "fg")
  {
    return std::make_shared<ForegroundCommand>(ForegroundCommand(org_cmd, args));
  }
  else if (args[0] == "bg")
  {
    return std::make_shared<BackgroundCommand>(BackgroundCommand(org_cmd, args));
  }
  else if (args[0] == "quit")
  {
    return std::make_shared<QuitCommand>(QuitCommand(org_cmd, args));
  }
  else if (args[0] == "kill")
  {
    return std::make_shared<KillCommand>(KillCommand(org_cmd, args));
  }
  else if (args[0] == "setcore")
  {
    return std::make_shared<SetcoreCommand>(SetcoreCommand(org_cmd, args));
  }
  else if (args[0] == "getfileinfo")
  {
    return std::make_shared<GetFileTypeCommand>(GetFileTypeCommand(org_cmd, args));
  }
  else if (args[0] == "chmod")
  {
    return std::make_shared<ChmodCommand>(ChmodCommand(org_cmd, args));
  }
  else if (args[0] == "timeout")
  {
  }
  else
  {
    // external commands
    return std::make_shared<ExternalCommand>(org_cmd, args);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  try
  {
    std::shared_ptr<Command> cmd = CreateCommand(cmd_line);
    if (cmd)
      cmd->execute();

    // BuiltInstd::shared_ptr<Command>tmp_cmd = dynamic_cast<BuiltInstd::shared_ptr<Command>>(cmd);
    // delete tmp_cmd;
  }
  catch (CommandException &e)
  {
  }
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

chpromptCommand::chpromptCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  prompt = args.size() > 1 ? args[1] : string("smash");
};

ShowPidCommand::ShowPidCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  smash_pid = SmallShell::getInstance().getPidSmash();
}

GetCurrDirCommand::GetCurrDirCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (!getcwd(NULL, 0))
  {
    throw DefaultError(cmd_str);
  }
  pwd = getcwd(NULL, 0);
}

ChangeDirCommand::ChangeDirCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (args.size() > 2)
  {
    throw TooManyArguments(args[0]);
  }
  if (args.size() < 2)
  {
    throw DefaultError(cmd_str);
  }
  new_path = args[1];
  if (new_path == "-" && SmallShell::getInstance().getLastWd() == "")
  {
    throw OldPwdNotSet(args[0]);
  }
  else if (new_path == "-")
  {
    new_path = SmallShell::getInstance().getLastWd();
  }
}

JobsCommand::JobsCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
}

ForegroundCommand::ForegroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (args.size() == 1)
  {
    int res;
    SmallShell::getInstance().getJobs().getLastJob(&res);
    if (res == -1)
    {
      perror("smash error: fg: jobs list is empty");
    }
    // cout << "res is: " << res << endl;
    job_to_fg = res;
  }
  else
  {
    try
    {
      job_to_fg = stoi(args[1]);
    }
    catch (std::exception &e)
    {
      throw InvalidArguments(args[0]);
    }
  }
  // else
  //   throw InvalidArguments(args[0]);
}

BackgroundCommand::BackgroundCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (args.size() == 1)
  {
    int res;
    if (SmallShell::getInstance().getJobs().getLastStoppedJob(&res))
      job_to_bg = res;
    else
      throw NoStopedJobs(args[0]);
  }
  else
  {
    try
    {
      job_to_bg = stoi(args[1]);
    }
    catch (std::exception &e)
    {
      throw InvalidArguments(args[0]);
    }

    if (!SmallShell::getInstance().getJobs().getJobById(job_to_bg))
      throw JobDoesNotExist(args[0], job_to_bg);

    if (!SmallShell::getInstance().getJobs().getJobById(job_to_bg)->isStopped())
      throw AlreadyRunningInBackground(args[0], job_to_bg);

    if (args.size() > 2)
      throw InvalidArguments(args[0]);
  }
}

QuitCommand::QuitCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
}

KillCommand::KillCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (args.size() >= 3)
  {
    try
    {
      signal_num = stoi(args[1].substr(1));
      job_to_kill = stoi(args[2]);
    }
    catch (const std::exception &e)
    {
      throw InvalidArguments(args[0]);
    }
  }
  else
    throw InvalidArguments(args[0]);
}

ExternalCommand::ExternalCommand(string cmd_line, vector<string> args, pid_t pid) : Command(cmd_line, args, pid)
{
}

SetcoreCommand::SetcoreCommand(const string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{

  try
  {
    job_to_setcore = stoi(args[1]);
    core_num = stoi(args[2]);
  }
  catch (const std::exception &e)
  {
    throw InvalidArguments(args[0]);
  }

  int max_core = std::thread::hardware_concurrency();

  if (SmallShell::getInstance().getJobs().getJobById(job_to_setcore) == nullptr)
  {
    throw JobDoesNotExist(args[0], job_to_setcore);
  }
  if (core_num < 0 || core_num > max_core)
  {
    throw InvalidCoreNumber(args[0]);
  }
}

GetFileTypeCommand::GetFileTypeCommand(const string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  if (args.size() != 2)
  {
    throw InvalidArguments(args[0]);
  }
}

ChmodCommand::ChmodCommand(string cmd_line, vector<string> args, pid_t pid) : BuiltInCommand(cmd_line, args, pid)
{
  // check input validity
  if (args.size() != 3)
  {
    throw InvalidArguments(args[0]);
  }

  // Parse the std::shared_ptr<mode from the command line arguments
  char *endptr;
  strtol(args[1].c_str(), &endptr, 8);

  // Check that the std::shared_ptr<mode was valid
  if (*endptr != '\0' || errno == ERANGE)
  {
    throw InvalidArguments(args[0]);
  }

  struct stat file_stat;
  // Retrieve the current file mode of the file
  if (stat(args[2].c_str(), &file_stat) == -1)
  {
    throw InvalidArguments(args[0]);
  }
}

/************** !!!!executes!!!! ******************/

void chpromptCommand::execute()
{
  // cout << this->prompt << endl;
  // cout << SmallShell::getInstance().getPrompt() << endl;
  SmallShell::getInstance().setPrompt(this->prompt);
}
void ShowPidCommand::execute()
{

  cout << "smash pid is " << smash_pid << endl;
  // cout << "smash pid is " << SmallShell::getInstance().getPidSmash() << endl;
  // SmallShell::getInstance().setPrompt(this->prompt);
}

void GetCurrDirCommand::execute()
{
  cout << pwd << endl;
}
void ChangeDirCommand::execute()
{
  string temp = getcwd(NULL, 0);
  if (chdir(new_path.c_str()) == -1)
  {
    perror("smash error: chdir failed");
    return;
  }
  SmallShell::getInstance().setLastWd(temp);
}
void JobsCommand::execute()
{
  SmallShell::getInstance().printJobs();
}

void ForegroundCommand::execute()
{
  JobsList::JobEntry *the_job = SmallShell::getInstance().getJobs().getJobById(job_to_fg);
  if (the_job == nullptr)
    throw JobDoesNotExist(args[0], job_to_fg);

  if (args.size() > 2)
    throw InvalidArguments(args[0]);

  std::shared_ptr<Command> cmd = SmallShell::getInstance().getJobs().getJobById(job_to_fg)->getCmd();
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
  SmallShell::getInstance().setCurrentCmd(nullptr);
}

void BackgroundCommand::execute()
{
  cout << SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getCommand()
       << " : " << SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getPid() << endl;
  SmallShell::getInstance().getJobs().getJobById(job_to_bg)->setStopped(false);

  if (kill(SmallShell::getInstance().getJobs().getJobById(job_to_bg)->getPid(), SIGCONT) == -1)
  {
    perror("smash error: kill failed");
  }
}

void QuitCommand::execute()
{
  SmallShell::getInstance().getJobs().removeFinishedJobs();
  if (args.size() > 1 && args[1] == "kill")
  {
    cout << "smash: sending SIGKILL signal to " << SmallShell::getInstance().getJobs().jbs_map.size() << " jobs:" << endl;
    // print jobs to kill
    SmallShell::getInstance().getJobs().killprintJobsList();
    SmallShell::getInstance().getJobs().killAllJobs(false);
  }
  exit(0);
}

void KillCommand::execute()
{
  JobsList::JobEntry *job = SmallShell::getInstance().getJobs().getJobById(job_to_kill);
  if (!job)
  {
    throw JobDoesNotExist(args[0], job_to_kill);
  }
  if (args.size() != 3)
  {
    throw InvalidArguments(args[0]);
  }

  pid_t pid_to_kill = job->getPid();

  if (SIGCONT == signal_num)
  {
    job->setStopped(false);
  }
  if (signal_num == SIGSTOP)
  {
    job->setStopped(true);
  }
  if (kill(pid_to_kill, signal_num) == -1)
  {
    perror("smash error: kill failed");
  }
  else
  {
    //
    // SmallShell::getInstance().getJobs().printJobsList();
    if (signal_num == SIGINT || signal_num == SIGKILL)
    {
      SmallShell::getInstance().getJobs().removeJobById(job_to_kill);
    }

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
        // char_args[i] = std::shared_ptr<char[args[i].length() + 1];
        // strcpy(char_args[i], args[i].c_str());
      }
      char_args.push_back(NULL);
      if (execvp(args[0].c_str(), char_args.data()) == -1) // i think its shoulnd be execvp (maybe execv)
      {
        // for (int i = 0; i < args.size(); i++)
        // {
        //   delete char_args[i];
        // }
        perror("smash error: execl failed");
        exit(1);
      }
      // for (int i = 0; i < args.size(); i++)
      // {
      //   delete char_args[i];
      // }
    }
  }
  else // father
  {
    this->pid = temp_pid;
    if (_isBackgroundComamnd(cmd_str))
    {
      SmallShell::getInstance().getJobs().addJob(shared_from_this(), false);
    }
    else
    {
      SmallShell::getInstance().setCurrentCmd(shared_from_this());
      waitpid(temp_pid, NULL, WUNTRACED);
      SmallShell::getInstance().setCurrentCmd();
      // delete this;

      // need to delete ?
    }
  }
}

void RedirectionCommand::execute()
{
  int stdout_copy = dup(STDOUT_FILENO);
  close(STDOUT_FILENO);
  int fd;
  if (append)
  {
    fd = open(args[1].c_str(), O_WRONLY | O_CREAT | O_APPEND, 0655);
  }
  else
  {
    fd = open(args[1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0655);
  }
  if (fd == -1)
  {
    dup2(stdout_copy, STDOUT_FILENO);
    perror("smash error: open failed");
    throw CommandException();
  }
  try
  {
    std::shared_ptr<Command> cmd = SmallShell::getInstance().CreateCommand(args[0].c_str());
    if (!cmd)
    {
      dup2(stdout_copy, STDOUT_FILENO);
      close(stdout_copy);
      throw InvalidArguments(args[0]);
    }
    cmd->execute();
    // delete cmd;
  }
  catch (CommandException &e)
  {
  }
  if (close(fd) == -1)
  {
    perror("smash error: close failed");
  }
  dup2(stdout_copy, STDOUT_FILENO);
  close(stdout_copy);
}

void PipeCommand::execute()
{
  int in_copy = dup(STDIN_FILENO);
  int out_copy = dup(STDOUT_FILENO);
  int err_copy = dup(STDERR_FILENO);

  if (in_copy == -1 || out_copy == -1 || err_copy == -1)
  {
    perror("smash error: dup failed");
    throw CommandException();
  }

  int fd[2];
  if (pipe(fd) == -1)
  {
    perror("smash error: pipe failed");
    throw CommandException();
  }
  pid_t pid1 = fork();
  if (pid1 == -1)
  {
    close(fd[0]);
    close(fd[1]);
    dup2(in_copy, STDIN_FILENO);
    dup2(out_copy, STDOUT_FILENO);
    dup2(err_copy, STDERR_FILENO);
    perror("smash error: fork failed");
    throw CommandException();
  }
  else if (pid1 == 0)
  {
    close(fd[0]);
    pipe_err ? dup2(fd[1], STDERR_FILENO) : dup2(fd[1], STDOUT_FILENO);
    std::shared_ptr<Command> cmd1 = SmallShell::getInstance().CreateCommand(args[0].c_str());
    if (!cmd1)
    {
      close(fd[1]);
      dup2(in_copy, STDIN_FILENO);
      dup2(out_copy, STDOUT_FILENO);
      dup2(err_copy, STDERR_FILENO);
      throw InvalidArguments(args[0]);
    }
    cmd1->execute();
    // delete cmd1;
    pipe_err ? dup2(err_copy, STDERR_FILENO) : dup2(out_copy, STDOUT_FILENO);
    // close(fd[1]); //is it ok to close here?
    exit(0);
  }
  else
  {
    pid_t pid2 = fork();
    if (pid2 == -1)
    {
      close(fd[0]);
      close(fd[1]);
      dup2(in_copy, STDIN_FILENO);
      dup2(out_copy, STDOUT_FILENO);
      dup2(err_copy, STDERR_FILENO);
      perror("smash error: fork failed");
      throw CommandException();
    }
    else if (pid2 == 0)
    {
      close(fd[1]);
      dup2(fd[0], STDIN_FILENO);
      std::shared_ptr<Command> cmd2 = SmallShell::getInstance().CreateCommand(args[1].c_str());
      if (!cmd2)
      {
        close(fd[0]);
        dup2(in_copy, STDIN_FILENO);
        dup2(out_copy, STDOUT_FILENO);
        dup2(err_copy, STDERR_FILENO);
        throw InvalidArguments(args[1]);
      }
      cmd2->execute();
      // delete cmd2;
      dup2(in_copy, STDIN_FILENO);
      // close(fd[0]); //is it ok to close here?
      exit(0);
    }
    else
    {
      close(fd[0]);
      close(fd[1]);
      waitpid(pid1, NULL, WUNTRACED);
      waitpid(pid2, NULL, WUNTRACED);
      dup2(in_copy, STDIN_FILENO);
      dup2(out_copy, STDOUT_FILENO);
      dup2(err_copy, STDERR_FILENO);
    }
  }
}

void SetcoreCommand::execute()
{
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(core_num, &set);
  JobsList::JobEntry *job_ent = SmallShell::getInstance().getJobs().getJobById(job_to_setcore);
  if (!job_ent)
  {
    throw JobDoesNotExist(args[0], job_to_setcore);
  }
  if (args.size() != 3)
  {
    throw InvalidArguments(args[0]);
  }

  if (sched_setaffinity(job_ent->getPid(), sizeof(set), &set) == -1)
  {
    perror("smash error: sched_setaffinity failed");
    throw CommandException();
  }
}

void GetFileTypeCommand::execute()
{
  string path = args[1].c_str();

  struct stat file_info;
  if (stat(path.c_str(), &file_info) == -1)
  {
    throw InvalidArguments(cmd_str);
  }
  cout << path << "'s type is";
  if (S_ISREG(file_info.st_mode))
  {
    std::cout << " \"regular file\" ";
  }
  else if (S_ISDIR(file_info.st_mode))
  {
    std::cout << " \"directory\" ";
  }
  else if (S_ISCHR(file_info.st_mode))
  {
    std::cout << " \"character device\" ";
  }
  else if (S_ISBLK(file_info.st_mode))
  {
    std::cout << " \"block device\" ";
  }
  else if (S_ISFIFO(file_info.st_mode))
  {
    std::cout << " \"FIFO\" ";
  }
  else if (S_ISLNK(file_info.st_mode))
  {
    std::cout << " \"symbolic link\" ";
  }
  else if (S_ISSOCK(file_info.st_mode))
  {
    std::cout << " \"socket\" ";
  }
  else
  {
    throw InvalidArguments(args[0]);
  }
  cout << "and takes up " << file_info.st_size << " bytes" << endl;
}

void ChmodCommand::execute()
{
  // Set the updated file mode using chmod
  char *endptr;
  long new_mode = strtol(args[1].c_str(), &endptr, 8);
  if (chmod(args[2].c_str(), new_mode) == -1)
  {
    throw InvalidArguments(args[0]);
  }
}

/************** !!!!other-implements!!!! ******************/

void JobsList::removeFinishedJobs()
{
  // for (auto &[key, job] : jbs_map)
  // {

  //   auto wait_stat = waitpid(job.getPid(), NULL, WNOHANG);
  //   if (wait_stat == -1)
  //   {
  //     perror("smash error: waitpid failed");
  //   }
  //   else if (wait_stat != 0)
  //   {
  //     jbs_map.erase(key);
  //   }
  // }

  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    auto wait_stat = waitpid(it->second.getPid(), NULL, WNOHANG);
    if (wait_stat == 0)
    {
      it++;
    }
    else if (wait_stat == -1)
    {
      perror("smash error: waitpid failed");
    }
    else
    {
      // delete it->second.getCmd();
      it = jbs_map.erase(it);
    }
  }
}

void JobsList::printJobsList()
{
  removeFinishedJobs();
  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    time_t now = time(NULL);
    // int seconds = difftime(now, job.getStartTime());
    cout << "[" << it->first << "] " << it->second.getCommand() << " : " << it->second.getPid() << " " << int(difftime(now, it->second.getStartTime())) << " secs";
    if (it->second.isStopped())
    {
      cout << " (stopped)";
    }
    cout << endl;
    it++;
  }
}

void JobsList::killprintJobsList()
{
  removeFinishedJobs();
  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    // time_t now = time(NULL);
    cout << it->second.getPid() << ": " << it->second.getCommand() << endl;
    it++;
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
  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    if (it->second.isStopped())
    {
      *jobId = it->first;
      res = &it->second;
    }
    it++;
  }
  return res;
}

JobsList::JobEntry *JobsList::getLastJob(int *jobId)
{
  if (jbs_map.empty())
  {
    *jobId = -1;
    return nullptr;
  }
  *jobId = jbs_map.rbegin()->first;
  return &jbs_map.rbegin()->second;
}

void JobsList::addJob(std::shared_ptr<Command> cmd, bool isStopped)
{
  removeFinishedJobs();
  if (cmd->getJobId() != -1)
  {
    jbs_map.insert({cmd->getJobId(), JobEntry(cmd->getJobId(), isStopped, cmd)});
    return;
  }
  int max_id = 0;
  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    if (it->first > max_id)
    {
      max_id = it->first;
    }
    it++;
  }
  // JobEntry job(max_id, isStopped, cmd);
  cmd->setJobId(max_id + 1);
  jbs_map.insert({max_id + 1, JobEntry(max_id, isStopped, cmd)});
  // JobEntry job(cmd, isStopped);
  // jbs_map.insert({job.getPid(), job});
}

JobsList::JobEntry::JobEntry(int id, bool isStopped, std::shared_ptr<Command> cmd) : job_id(id), is_stopped(isStopped), cmd(cmd)
{
  // this->pid = cmd->getPid();
  this->start_time = time(NULL);
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  if (jbs_map.count(jobId) == 0)
  {
    return nullptr;
  }

  return &jbs_map.at(jobId);
}

void JobsList::removeJobById(int jobId)
{
  jbs_map.erase(jobId);
}

void JobsList::killAllJobs(bool print)
{
  for (auto it = jbs_map.begin(); it != jbs_map.end();)
  {
    if (kill(it->second.getPid(), SIGKILL) == -1)
    {
      perror("smash error: kill failed");
    }
    else if (print)
    {
      cout << "signal number " << SIGKILL << " was sent to pid " << it->second.getPid() << endl;
    }
    it++;
  }
}
