#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <unistd.h>
#include <memory>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;

class Command : public std::enable_shared_from_this<Command>
{
protected:
  std::string cmd_str; // without changes
  std::vector<string> args;
  pid_t pid;
  int job_id;

public:
  Command(string cmd_line, vector<string> args, pid_t pid = -1, int job_id = -1) : cmd_str(cmd_line), args(args), pid(pid), job_id(job_id){};
  virtual ~Command(){};
  virtual void execute() = 0;
  std::string getCmdStr() { return cmd_str; };
  pid_t getPid() { return pid; }
  int getJobId() { return job_id; }
  void setJobId(int new_job_id) { job_id = new_job_id; }
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
  ExternalCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  bool pipe_err;
  // TODO: Add your data members
public:
  // PipeCommand(const char *cmd_line);
  explicit PipeCommand(const string cmd_line, bool pipe_err, vector<string> args, pid_t pid = -1) : Command(cmd_line, args, pid), pipe_err(pipe_err){};
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
  // string cmd_lft;
  // string file_name;
  bool append;

public:
  // explicit RedirectionCommand(const char *cmd_line);
  explicit RedirectionCommand(const string cmd_line, bool append, vector<string> args, pid_t pid = -1) : Command(cmd_line, args, pid), append(append){
                                                                                                                                           // cmd_lft = cmd_line.substr(0, cmd_line.find_first_of(">")); // delete &?
                                                                                                                                           // file_name = args.back();                                   // cmd_line.substr(cmd_line.find_first_of(">") + 1 + append, cmd_line.size());
                                                                                                                                           // args.pop_back();
                                                                                                                                       };
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
  chpromptCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~chpromptCommand(){};
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand
{
private:
  string new_path;

public:
  // TODO: Add your data members public:
  ChangeDirCommand(string cmd_line, vector<string> args, pid_t pid = -1);
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
  ShowPidCommand(string cmd_line, vector<string> args, pid_t pid = -1);
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
    int job_id;
    bool is_stopped;
    time_t start_time;
    std::shared_ptr<Command> cmd;

  public:
    JobEntry(int job_id, bool is_stopped, std::shared_ptr<Command> cmd);
    // int getJobId() const { return _job_id; }
    pid_t getPid() const { return cmd->getPid(); }
    string getCommand() const { return cmd->getCmdStr(); }
    std::shared_ptr<Command> getCmd() { return cmd; }
    void setCmd(std::shared_ptr<Command> cmd) { cmd = cmd; }
    bool isStopped() const { return is_stopped; }
    void setStopped(bool new_is_stopped) { is_stopped = new_is_stopped; }
    time_t getStartTime() const { return start_time; }
  };
  map<int, JobEntry> jbs_map;

public:
  JobsList();
  ~JobsList() = default;
  void addJob(std::shared_ptr<Command> cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs(bool print = true);
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  void killprintJobsList();
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
  // int job_id;
  // JobsList::JobEntry *job;
  int job_to_fg;

public:
  ForegroundCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
private:
  int job_to_bg;
  // TODO: Add your data members
public:
  BackgroundCommand(string cmd_line, vector<string> args, pid_t pid = -1);
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
  ChmodCommand(string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand
{

public:
  GetFileTypeCommand(const string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand
{
  // TODO: Add your data members
  int job_to_setcore;
  int core_num;

public:
  SetcoreCommand(const string cmd_line, vector<string> args, pid_t pid = -1);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
  int job_to_kill;
  int signal_num;

public:
  KillCommand(const string cmd_line, vector<string> args, pid_t pid = -1);
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell
{
private:
  // TODO: Add your data members
  std::shared_ptr<Command> curr_cmd;
  std::string smash_name;
  JobsList jobs_list;
  string last_wd;

  //   Command *curr_cmd;
  // std::string smash_name;
  // JobsList jobs_list;
  // string last_wd;
  SmallShell();

public:
  std::shared_ptr<Command> CreateCommand(const char *cmd_line);
  void printJobs();
  JobsList &getJobs() { return jobs_list; }
  pid_t getPidSmash() { return getpid(); };
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    // std::cout << "in getInstance" << endl; // delete this
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    // instance.setPrompt();
    return instance;
  }
  std::string getPrompt() { return smash_name; };
  void setPrompt(std::string new_smash_name = string("smash"))
  {
    smash_name = new_smash_name;
  };
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  void setCurrentCmd(std::shared_ptr<Command> cmd = nullptr) { curr_cmd = cmd; };
  std::shared_ptr<Command> getCurrentCmd() { return curr_cmd; };
  void removeJobById(int jobId);
  // TODO: add extra methods as needed
  string getLastWd() { return last_wd; };
  void setLastWd(string new_last_wd) { last_wd = new_last_wd; };
};

class CommandException : public std::exception
{
};
class TooManyArguments : public CommandException
{
private:
  string cmd_line;

public:
  TooManyArguments(const string &cmd) : cmd_line(cmd)
  {
    cerr << "smash error: " << cmd_line << ": too many arguments" << endl;
  }
};

class OldPwdNotSet : public CommandException
{
private:
  string cmd_line;

public:
  OldPwdNotSet(const string &cmd) : cmd_line(cmd)
  {
    cerr << "smash error: " << cmd_line << ": OLDPWD not set" << endl;
  }
};

class DefaultError : public CommandException
{
private:
  string cmd_line;

public:
  DefaultError(const string &cmd) : cmd_line(cmd)
  {
    cerr << "smash error:>" << cmd_line << endl;
  }
};

class JobsListIsEmpty : public CommandException
{
private:
  string _cmd_line;

public:
  JobsListIsEmpty(const string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": jobs list is empty" << endl;
  }
};

class InvalidArguments : public CommandException
{
private:
  string _cmd_line;

public:
  InvalidArguments(const string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": invalid arguments" << endl;
  }
};

class JobDoesNotExist : public CommandException
{
private:
  string cmd_line;
  int job_id;

public:
  JobDoesNotExist(const string cmd, int job_id) : cmd_line(cmd), job_id(job_id)
  {
    cerr << "smash error: " << cmd_line << ": job-id " << job_id << " does not exist" << endl;
  }
};

class AlreadyRunningInBackground : public CommandException
{
private:
  string _cmd_line;
  int _job_id;

public:
  AlreadyRunningInBackground(const string cmd, int job_id) : _cmd_line(cmd), _job_id(job_id)
  {
    cerr << "smash error: " + _cmd_line + ": job-id " << _job_id << " is already running in the background" << endl;
  }
};

class NoStopedJobs : public CommandException
{
private:
  string cmd_line;

public:
  NoStopedJobs(const string &cmd) : cmd_line(cmd)
  {
    cerr << "smash error: " << cmd_line << ": there is no stopped jobs to resume" << endl;
  }
};

class InvalidCoreNumber : public CommandException
{
private:
  string _cmd_line;

public:
  InvalidCoreNumber(const string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": invalid core number" << endl;
  }
};

#endif // SMASH_COMMAND_H_
