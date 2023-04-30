#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string>
#include <iostream>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using std::endl;
using std::map;
using std::string;
using std::vector;

class Command
{
  std::string cmd_str; // without changes
  std::vector<string> args;
  pid_t pid;

public:
  Command(string cmd_line, vector<string> args, pid_t pid = -1) : cmd_str(cmd_line), args(args), pid(pid){};
  virtual ~Command(){};
  virtual void execute() = 0;
  std::string getCmdStr() { cmd_str; };
  pid_t getPid() { pid; }
  // virtual void prepare();
  // virtual void cleanup();
  //  TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(string cmd_line, vector<string> args, pid_t pid = -1) : Command(cmd_line, args, pid){};
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // TODO: Add your data members
public:
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

class chpromptCommand : public BuiltInCommand
{
private:
  string prompt;

public:
  chpromptCommand(string cmd_line, vector<string> args, pid_t pid = -1) : BuiltInCommand(cmd_line, args, pid)
  {
    prompt = args.size() > 1 ? args[1] : string("smash");
  };
  virtual ~chpromptCommand(){};
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand
{
private:
public:
  // TODO: Add your data members public:
  ChangeDirCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  ChangeDirCommand(const char *cmd_line, char **plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
private:
  string pwd;

public:
  GetCurrDirCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
private:
  pid_t smash_pid;

public:
  ShowPidCommand(string cmd_line, vector<string> args, pid_t pid = -1) : BuiltInCommand(cmd_line, args, pid)
  {
    std::cout << "in constructor shotpidcommand\n";
    // smash_pid = SmallShell::getInstance().getPidSmash();
  };
  ShowPidCommand(const char *cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  QuitCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList
{
public:
  class JobEntry
  {
    // TODO: Add your data members
    // int _job_id;
    bool is_stopped;
    time_t start_time;
    Command *cmd;

  public:
    JobEntry(int job_id, bool is_stopped, Command *cmd);
    // int getJobId() const { return _job_id; }
    pid_t getPid() const { return cmd->getPid(); }
    string getCommand() const { return cmd->getCmdStr(); }
    Command *getCmd() { return cmd; }
    void setCmd(Command *cmd) { cmd = cmd; }
    bool isStopped() const { return is_stopped; }
    void setStopped(bool new_is_stopped) { is_stopped = new_is_stopped; }
    time_t getStartTime() const { return start_time; }
  };
  map<int, JobEntry> jbs_map;
  // TODO: Add your data members
public:
  JobsList() = default;
  ~JobsList() = default;
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  ForegroundCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  BackgroundCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  BackgroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand
{
  /* Bonus */
  // TODO: Add your data members
public:
  explicit TimeoutCommand(const char *cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  GetFileTypeCommand(const char *cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  SetcoreCommand(const char *cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  KillCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell
{
private:
  // TODO: Add your data members
  JobsList jobs_list;
  std::string smash_name;
  SmallShell();

public:
  Command *CreateCommand(const char *cmd_line);
  void printJobs();
  pid_t getPidSmash() { return getpid(); };
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    instance.setPrompt();
    return instance;
  }
  std::string getPrompt() { return smash_name; };
  void setPrompt(std::string new_smash_name = string("smash"))
  {
    smash_name = new_smash_name;
  };
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
