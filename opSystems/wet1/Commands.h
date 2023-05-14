#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <time.h>
#include <list>
#include <stack>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
class Jobslist;

class Command {
protected:
  char** argv;
  int numOfArgs;
public:
  Command(const char* cmd_line);
  virtual ~Command()=default;
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
  public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  private:
    bool backGroundFlag;
    const std::string original;
  public:
    ExternalCommand(const char* cmd_line, bool bgFlag, const std::string& copy);
    virtual ~ExternalCommand() {}
    void execute() override;
    virtual void printName() const;
    int getPID() const;
};

class PipeCommand : public Command {
  bool errorFlag;
  Command* command1Ptr;
  Command* command2Ptr;
  public:
    PipeCommand(const char* cmd_line, bool errorPipe);
    virtual ~PipeCommand() {}
    void execute() override;
};


class RedirectionCommand : public Command {
  private:
    bool appendFlag;
  public:
    explicit RedirectionCommand(const char* cmd_line, bool toAppend = false);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChPromptCommand : public BuiltInCommand{
  public:
      ChPromptCommand(const char* cmd_line);
      virtual ~ChPromptCommand() {}
      void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
  public:
      ChangeDirCommand(const char* cmd_line);
      virtual ~ChangeDirCommand() {}
      void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
    public:
      GetCurrDirCommand(const char* cmd_line);
      virtual ~GetCurrDirCommand() {}
      void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
    public:
      ShowPidCommand(const char* cmd_line);
      virtual ~ShowPidCommand() {}
      void execute() override;
};

class QuitCommand : public BuiltInCommand {
  Jobslist* jobsListPtr;
  public:
    QuitCommand(const char* cmd_line);
    //QuitCommand(const QuitCommand&) = delete;
    virtual ~QuitCommand() {}
    void execute() override;
};

class JobsList{
  public:
    class JobEntry {
        public:
            JobEntry(ExternalCommand* cmd, int _jobId, int pid, bool isStopped = false);
            int getJobId() const;
            bool isStopped() const;
            void printJob() const;
            int getPID() const;
            void stop();
            void continueJob();
        private:
          ExternalCommand* cmdPtr;
          int jobId;
          int processId;
          bool stoppedFlag;
          time_t startTime;
    };
    bool isStopped(const JobEntry* jobPtr){
      return jobPtr->isStopped();
    }
  private:
    std::list<JobEntry*> jobsVector;
    int maxJobId;
  public:
    JobsList();
    ~JobsList();
    void addJob(ExternalCommand* cmd, int pid);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry* getJobById(int jobId);
    void removeJobById(int jobId);
    void removeJobByProcessID(int pid);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob();
    JobEntry *getMaxIdJob();
    bool isEmpty()const;
    void quitCmd();
    int getLastJobsPID() const;
  };

class JobsCommand : public BuiltInCommand {
  // TODO: Add your data members
  public:
    JobsCommand(const char* cmd_line);
    virtual ~JobsCommand() {}
    void execute() override;
  };

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public Command {
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public Command {
 public:
  GetFileTypeCommand(const char* cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public Command {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
private:
  std::string prompt;
  JobsList* jobsListPtr;
  char* previousWD;
  int numOfProcesses;
  int mostRecentPID;
  ExternalCommand* mostRecentCmd;
  SmallShell();
public:
  void updatePreviousWD(char* newPath);
  char*& getPreviousWD();
  Command* CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell() = default;
  void executeCommand(const char* cmd_line);
  void printPid(int pid);
  void setPrompt(const std::string &newPrompt);
  void printJobs() const;
  int getNumOfProcesses() const;
  void incrementNum();
  void quitCmd();
  JobsList::JobEntry* getJobByID(int id) const;
  JobsList::JobEntry* getMaxJob() const;
  JobsList::JobEntry* getLastStoppedJob() const;
  void addJob(ExternalCommand* cmd, int pid);
  bool jobListIsEmpty() const;
  std::string getPrompt() const;
  void setMostRecentPID(int pid);
  int getMostRecentPID() const;
  void SetMostRecentCommand(ExternalCommand* cmdPtr);
  ExternalCommand* getMostRecentCommand() const;
  int getLastJobsPID() const;
};
#endif //SMASH_COMMAND_H_
