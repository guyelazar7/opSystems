#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
//#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() :prompt("smash"), numOfProcesses(0), previousWD(NULL){}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  if(firstWord.compare("chprompt") == 0){
    return new ChPromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if(firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line, &previousWD);
  }
  else if(firstWord.compare("jobs") == 0){
    return new JobsCommand(cmd_line,jobsListPtr);
  }
  else if(firstWord.compare("fg") == 0){
    return new ForegroundCommand(cmd_line,jobsListPtr);
  }
  else if(firstWord.compare("bg") == 0){
    return new BackgroundCommand(cmd_line,jobsListPtr);
  }
  else if(firstWord.compare("quit") == 0){
    return new QuitCommand(cmd_line,jobsListPtr);
  }
  else if(firstWord.compare("kill") == 0){
    return new KillCommand(cmd_line,jobsListPtr);
  }
  else {
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

//-----------------------------------our smash functions-------------------------//

void SmallShell::setPrompt(const string &newPrompt){
    prompt = newPrompt;
};

void SmallShell::printPid(int pid){
  cout<<pid<<endl;
}

void SmallShell::updatePreviousWD(){
  if( getcwd(previousWD, sizeof(previousWD)) == NULL){
    perror("chdir() system call fails");
  }
}

void SmallShell::updatePreviousWD(char* newPath){
  previousWD = newPath;
}

char*& SmallShell::getPreviousWD(){
  return previousWD;
}

void SmallShell::printJobs() const{
  jobsListPtr->printJobsList();
}
//--------------------------------job list-----------------------------//

void JobsList::JobEntry::printJob() const {
    cout<<"["<<jobId<<"] ";
    cmdPtr->printName();
    cout<<":"<<cmdPtr->getPid()<<" ";
    //add seconds elapsed
    if(isStopped())
        cout<<"(stopped)"<<endl;
    else
        cout<<endl;
}


JobsList::JobsList() : jobsVector(), maxJobId(0){}

void JobsList::addJob(Command* cmd, bool isStopped = false){
    jobsVector.push_back(new JobEntry(cmd, ++maxJobId, 0, isStopped));
}
void JobsList::killAllJobs(){
    for(JobEntry*& element : jobsVector){
        kill(element);
    }
}
JobsList::JobEntry* JobsList::getJobById(int jobId){
    for(JobEntry*& element : jobsVector){
        if(element->getJobId() == jobId)
            return element;
    }
    return nullptr;
}
void JobsList::removeJobById(int jobId){
    for (vector<JobEntry*>::iterator it = jobsVector.begin(); it != jobsVector.end(); ++it ) { 
        if ((*it)->getJobId() == jobId){
            jobsVector.erase(it);
            return;
        }   
    }
}
void JobsList::removeFinishedJobs(){
    remove_if(jobsVector.begin(), jobsVector.end(), isStopped);
}
int JobsList::JobEntry::getJobId() const{
    return jobId;
}
bool JobsList::JobEntry::isStopped() const{
    return m_isStopped;
}
JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
    return jobsVector.back();
}
JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
    for (vector<JobEntry*>::reverse_iterator it = jobsVector.rbegin(); 
        it != jobsVector.rend(); ++it ) { 
          if ((*it)->isStopped())
            return *it;
    }
    return nullptr;
}

void JobsList::printJobsList(){
  for(JobEntry*& element : jobsVector){
    element->printJob();
  }
}


//----------------------------------------------command:-----------------------------------------------------------------------//

Command::Command(const char* cmd_line) : argv(new char*[1 + COMMAND_MAX_ARGS]){
  numOfArgs = _parseCommandLine(cmd_line, argv);
  pid = ++SmallShell::getInstance().numOfProcesses;
}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line){}

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line){}

PipeCommand::PipeCommand(const char* cmd_line) : Command(cmd_line){}

ChPromptCommand::ChPromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand(cmd_line){}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobListPtr ) : BuiltInCommand(cmd_line){}

ForegroundCommand::ForegroundCommand(const char* cmd_line,JobsList* jobListPtr ) : BuiltInCommand(cmd_line){}

BackgroundCommand::BackgroundCommand(const char*cmd_line,JobsList* jobListPtr ) : BuiltInCommand(cmd_line){}

QuitCommand::QuitCommand(const char* cmd_line,JobsList* jobListPtr ) : BuiltInCommand(cmd_line){}

KillCommand::KillCommand(const char* cmd_line,JobsList* jobListPtr) : BuiltInCommand(cmd_line){}

void ChPromptCommand::execute(){
  if (numOfArgs == 1){
    SmallShell::getInstance().setPrompt("smash");
  }
  else{
    SmallShell::getInstance().setPrompt(argv[1]);
  }
}

void ShowPidCommand::execute() {
    SmallShell::getInstance().printPid(pid);
}

void GetCurrDirCommand::execute(){
    char temp[100];
    cout<<getcwd(temp,100);
}
void ChangeDirCommand::execute(){
  if(strcmp(argv[1], "-") == 0){
      string temp(SmallShell::getInstance().getPreviousWD());
      SmallShell::getInstance().updatePreviousWD();
      chdir(temp.c_str());
      return;
  }
  if(strcmp(argv[1], "..") == 0){
      char temp[100];
      getcwd(temp,100);
      string tempStr=string(temp);
      int i=tempStr.length()-1;
      for(i; i>=0; --i){
        if (tempStr[i] == '\\') break;
      }
      string newCwd=tempStr.substr(0,i);
      if(newCwd.length() == 0){
        perror("smash error: chdir failed");//maybe will need to vhange message
        return;
      }
      SmallShell::getInstance().updatePreviousWD();
      chdir(newCwd.c_str());
      return;
  }
  else{
    char* temp = new char[100];
    getcwd(temp,100);
    if(chdir(argv[1]) == 0){
        SmallShell::getInstance().updatePreviousWD(temp);
    }
    else{
      perror("smash error: chdir failed");
    }
  }
}

void JobsCommand::execute(){
  SmallShell::getInstance().printJobs();
}