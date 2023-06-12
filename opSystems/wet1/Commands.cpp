#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iomanip>
#include "Commands.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
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

SmallShell::SmallShell() :prompt("smash"), jobsListPtr(new JobsList()), previousWD(NULL), numOfProcesses(0), 
            mostRecentPID(-1), mostRecentCmd(nullptr), fgFlag(false){}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
  const string copy=(string(cmd_line));
  bool backGroundFlag = _isBackgroundComamnd(cmd_line);
  _removeBackgroundSign((char*)cmd_line);
  string cmd_s = _trim(string(cmd_line));

  if(cmd_s.find(">") != string::npos){
    return new RedirectionCommand(cmd_line, cmd_s.find(">>") != string::npos);
  }
  if(cmd_s.find("|") != string::npos){
    return new PipeCommand(cmd_line, cmd_s.find("|&") != string::npos);
  }

  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  if(firstWord.compare("timeout") == 0){
    return new TimeoutCommand(cmd_line, copy, backGroundFlag);
  }
  else if(firstWord.compare("chmod") == 0){
    return new ChmodCommand(cmd_line);
  }
  else if(firstWord.compare("chprompt") == 0){
    return new ChPromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if(firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line);
  }
  else if(firstWord.compare("jobs") == 0){
    return new JobsCommand(cmd_line);
  }
  else if(firstWord.compare("fg") == 0){
    return new ForegroundCommand(cmd_line);
  }
  else if(firstWord.compare("bg") == 0){
    return new BackgroundCommand(cmd_line);
  }
  else if(firstWord.compare("quit") == 0){
    return new QuitCommand(cmd_line);
  }
  else if(firstWord.compare("kill") == 0){
    return new KillCommand(cmd_line);
  }
  else if(firstWord.compare("getfiletype")==0){
    return new GetFileTypeCommand(cmd_line);
  }
  else if(firstWord.compare("setcore")==0){
    return new SetcoreCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line, backGroundFlag, copy);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  jobsListPtr->removeFinishedJobs();
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

string SmallShell::getPrompt() const{
  return prompt;
}

void SmallShell::setMostRecentPID(int pid){
  mostRecentPID = pid;
}
int SmallShell::getMostRecentPID() const{
  return mostRecentPID;
}
void SmallShell::SetMostRecentCommand(ExternalCommand* cmdPtr){
  mostRecentCmd = cmdPtr;
}
ExternalCommand* SmallShell::getMostRecentCommand() const{
  return mostRecentCmd;
}

int SmallShell::getLastJobsPID() const{
  if(jobListIsEmpty()) return -1;
  return jobsListPtr->getLastJobsPID();
}

void SmallShell::removeJobByID(int pid){
  jobsListPtr->removeJobByID(pid);
}

JobsList::JobEntry* SmallShell::getFgJob(){
  return jobsListPtr->getJobByCmd(mostRecentCmd);
}
//-----------------------------------our smash functions-------------------------//

void SmallShell::setPrompt(const string &newPrompt){
    prompt = newPrompt;
}

void SmallShell::printPid(int pid){
  cout<<pid<<endl;
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

int SmallShell::getNumOfProcesses() const{
  return numOfProcesses;
}

void SmallShell::incrementNum(){
  ++numOfProcesses;
}

void SmallShell::quitCmd(){
  jobsListPtr->removeFinishedJobs();
  jobsListPtr->quitCmd();
}

JobsList::JobEntry* SmallShell::getJobByID(int id) const{
  return jobsListPtr->getJobById(id);
}

void SmallShell::addJob(ExternalCommand* cmd, int pid){
  jobsListPtr->addJob(cmd, pid);
}

bool SmallShell::jobListIsEmpty() const{
  return jobsListPtr->isEmpty();
}

JobsList::JobEntry* SmallShell::getMaxJob() const{
  return jobsListPtr->getMaxIdJob();
}
//--------------------------------job list-----------------------------//

void JobsList::JobEntry::printJob() const {
    cout<<"["<<jobId<<"] ";
    cmdPtr->printName();
    cout << " : " << processId << " ";
    cout << difftime(time(NULL),startTime) << " secs";
    if(stoppedFlag)
        cout<<" (stopped)"<<endl;
    else
        cout<<endl;
}
void JobsList::quitCmd(){
  cout<<"smash: sending SIGKILL signal to "<<jobsVector.size()<<" jobs:"<<endl;
  for(JobEntry*& element : jobsVector){
    cout<<element->getPID()<<": ";
    element->printOriginalLine();
    cout<<endl;
    if(kill(element->getPID(),SIGKILL)==-1)
      perror("smash error: kill failed");      
  }
}
int JobsList::JobEntry::getPID() const{
  return processId;
}

JobsList::JobEntry::JobEntry(ExternalCommand* cmd, int _jobId, int pid, bool isStopped):cmdPtr(cmd),
                            jobId(_jobId),processId(pid), stoppedFlag(isStopped),startTime(time(NULL)){}
JobsList::JobsList() : jobsVector(), maxJobId(0){}
void JobsList::addJob(ExternalCommand* cmd, int pid){
    removeFinishedJobs();
    jobsVector.push_back(new JobEntry(cmd, ++maxJobId, pid));
}
void JobsList::killAllJobs(){
    for(JobEntry*& element : jobsVector){
        if(kill(element->getPID(),SIGKILL)==-1)
         perror("smash error: kill failed");
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
    for (list<JobEntry*>::iterator it = jobsVector.begin(); it != jobsVector.end(); ++it ) { 
        if ((*it)->getJobId() == jobId){
            jobsVector.erase(it);
            return;
        }   
    }
}

void JobsList::removeJobByProcessID(int pid){
    for (list<JobEntry*>::iterator it = jobsVector.begin(); it != jobsVector.end(); ++it ) { 
        if ((*it)->getPID() == pid){
            jobsVector.erase(it);
            return;
        }   
    }
}
void JobsList::removeFinishedJobs(){
    if(jobsVector.empty()){
      return;
    }
    for(int pid = waitpid(-1, NULL, WNOHANG); pid > 0; pid = waitpid(-1, NULL, WNOHANG)){
        removeJobByProcessID(pid);
    }    
    for(list<JobEntry*>::reverse_iterator it = jobsVector.rbegin(); 
        it != jobsVector.rend();){
         if(kill((*it)->getPID(),0)!=0){
            it = list<JobEntry*>::reverse_iterator(jobsVector.erase(next(it).base()));
         }
         else
          ++it;
    }    
    maxJobId = jobsVector.empty() ? 0 : jobsVector.back()->getJobId();
}
int JobsList::JobEntry::getJobId() const{
    return jobId;
}

bool JobsList::isEmpty()const{
  return jobsVector.empty();
}
JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
    return jobsVector.back();
}
JobsList::JobEntry* JobsList::getLastStoppedJob(){
  for (list<JobEntry*>::reverse_iterator it = jobsVector.rbegin(); 
    it != jobsVector.rend(); ++it ) { 
      if ((*it)->isStopped())
        return *it;
  }
    return nullptr;
}
JobsList::JobEntry* JobsList::getMaxIdJob(){
  if(jobsVector.empty())
    return nullptr;
  return jobsVector.back();
}
void JobsList::printJobsList(){
  removeFinishedJobs();
  if(jobsVector.empty()) return;
  for(JobEntry*& element : jobsVector){
    element->printJob();
  }
}

JobsList::JobEntry* SmallShell::getLastStoppedJob() const{
  return jobsListPtr->getLastStoppedJob();
}

bool JobsList::JobEntry::isStopped() const{
  return stoppedFlag;
}

void JobsList::JobEntry::stop(){
  stoppedFlag = true;
}

void JobsList::JobEntry::continueJob(){
  stoppedFlag = false;
}

int JobsList::getLastJobsPID() const{
  return jobsVector.back()->getPID();
}

void JobsList::JobEntry::printOriginalLine(){
  cmdPtr->printOriginal();
}

void JobsList::removeJobByID(int pid){
  if(jobsVector.empty()) return;
  for(list<JobEntry*>::iterator it = jobsVector.begin(); it != jobsVector.end(); ++it){
    if((*it)->getPID() == pid){
      jobsVector.erase(it);
      return;
    }
  }
}

JobsList::JobEntry* JobsList::getJobByPID(int pid){
  if(jobsVector.empty()) return nullptr;
  for(JobEntry*& element : jobsVector){
    if(element->getPID() == pid) return element;
  }
  return nullptr;
}

JobsList::JobEntry* JobsList::getJobByCmd(ExternalCommand* cmd){
  if(jobsVector.empty()) return nullptr;
  for(JobEntry*& element : jobsVector){
    if(element->getCmd() == cmd) return element;
  }
  return nullptr;
}
//----------------------------------------------commandctors:-----------------------------------------------------------------------//

Command::Command(const char* cmd_line) : argv(new char*[1 + COMMAND_MAX_ARGS]){
  numOfArgs = _parseCommandLine(cmd_line, argv);
  SmallShell::getInstance().incrementNum();
}

BuiltInCommand::BuiltInCommand (const char* cmd_line) : Command (cmd_line){
  int lastLength = strlen(argv[numOfArgs-1]);
  bool backGroundFlag = (argv[numOfArgs-1][lastLength-1] == '&');
  if(backGroundFlag && lastLength == 1){
    argv[numOfArgs-1] = NULL;
    --numOfArgs;
  }
  else if(backGroundFlag) argv[numOfArgs-1][lastLength-1] = 0;
}

ExternalCommand::ExternalCommand(const char* cmd_line, bool bgFlag, const string& copy) : Command(cmd_line),
                  backGroundFlag(bgFlag), original(copy){}

PipeCommand::PipeCommand(const char* cmd_line, bool errorPipe): Command(cmd_line), errorFlag(errorPipe){
  string cmd(cmd_line);
  int position = cmd.find_first_of('|');
  command1Ptr = SmallShell::getInstance().CreateCommand(cmd.substr(0,position).c_str());
  int offset = 1 + (int) errorFlag;
  command2Ptr = SmallShell::getInstance().CreateCommand(cmd.substr(position + offset).c_str());
}

ChPromptCommand::ChPromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

JobsCommand::JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

ForegroundCommand::ForegroundCommand(const char* cmd_line ) : BuiltInCommand(cmd_line){}

BackgroundCommand::BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

QuitCommand::QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

KillCommand::KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

ChmodCommand::ChmodCommand(const char* cmd_line) : Command(cmd_line){}

GetFileTypeCommand::GetFileTypeCommand(const char* cmd_line) : Command(cmd_line){}

SetcoreCommand::SetcoreCommand(const char* cmd_line) : Command(cmd_line){}

RedirectionCommand::RedirectionCommand(const char* cmd_line, bool toAppend): Command(cmd_line), appendFlag(toAppend){
  string cmd(cmd_line);
  int position = cmd.find_first_of('>');
  command = SmallShell::getInstance().CreateCommand(cmd.substr(0,position).c_str());
  fileName = _trim(cmd.substr(position + 1 + (cmd[position+1] == '>')));
}


//----------------------------------------------execute:-----------------------------------------------------------------------//

void ChPromptCommand::execute(){ // done
  if (numOfArgs == 1){
    SmallShell::getInstance().setPrompt("smash");
  }
  else{
    SmallShell::getInstance().setPrompt(argv[1]);
  }
}
void ShowPidCommand::execute() { //done
    cout<<"smash pid is "<<getpid()<<endl;
}
void GetCurrDirCommand::execute(){ // done
    char temp[COMMAND_ARGS_MAX_LENGTH];
    cout<<getcwd(temp,COMMAND_ARGS_MAX_LENGTH)<<endl;
}
void ChangeDirCommand::execute(){ // done
  if(numOfArgs!=2){
    cerr<<"smash error: cd: too many arguments"<<endl;
    return;
  } 
  char* temp = new char[COMMAND_ARGS_MAX_LENGTH];
    getcwd(temp,COMMAND_ARGS_MAX_LENGTH);
  if(strcmp(argv[1],"-") == 0){
      if(SmallShell::getInstance().getPreviousWD()==NULL){
        cerr <<"smash error: cd: OLDPWD not set"<<endl;
        return;
      }
      char* newCwd = SmallShell::getInstance().getPreviousWD();
      if(chdir(newCwd) == 0){
          SmallShell::getInstance().updatePreviousWD(temp);
      }
  }
  else if(strcmp(argv[1], "..") == 0){
      string tempStr = string(temp);
      string newCwd = tempStr.substr(0,tempStr.find_last_of('/')+1);
      if(newCwd.length() == 0){
        cerr<<"smash error: chdir failed"<<endl;//maybe will need to vhange message
        return;
      }
      if(chdir(newCwd.c_str()) == 0){
          SmallShell::getInstance().updatePreviousWD(temp);//make sure the temp doesnt change
      }
      else{
        if (errno == ENOENT)
          cerr<<"smash error: chdir failed: No such file or directory"<<endl;
        else
          cerr<<"smash error: chdir failed"<<endl;
      }
      return;
  }
  else if(argv[1][0] == '/'){
    if(chdir(argv[1]) == 0){
        SmallShell::getInstance().updatePreviousWD(temp);
    }
    else{
        if (errno == ENOENT)
          cerr<<"smash error: chdir failed: No such file or directory"<<endl;
        else
          cerr<<"smash error: chdir failed"<<endl;
    }
  }
  else{
    if(chdir((string(temp) + '/' + string(argv[1])).c_str()) == 0){
          SmallShell::getInstance().updatePreviousWD(temp);//make sure the temp doesnt change
    }
    else{
        if (errno == ENOENT)
          cerr<<"smash error: chdir failed: No such file or directory"<<endl;
        else
          cerr<<"smash error: chdir failed"<<endl;
    }
  }
}
void ForegroundCommand::execute(){ // done
  JobsList::JobEntry* fgJob;
  SmallShell& smash = SmallShell::getInstance();
  if(numOfArgs == 1){
    fgJob = smash.getMaxJob();
    if(fgJob == nullptr){
      cerr<<"smash error: fg: jobs list is empty"<<endl;
      return;
    }
  }
  else if(numOfArgs == 2)
  {
    try{
      fgJob = smash.getJobByID(stoi(argv[1]));
    }catch(...){
      cerr<<"smash error: fg: invalid arguments"<<endl;
      return;
    }
    if(fgJob == nullptr){
      cerr<<"smash error: fg: job-id "<<argv[1]<<" does not exist"<<endl; 
      return;
    }
  }  
  else{
    cerr<<"smash error: fg: invalid arguments"<<endl;
    return;
  }
  fgJob->continueJob();
  kill(fgJob->getPID(), SIGCONT);
  smash.SetMostRecentCommand(fgJob->getCmd());
  fgJob->printOriginalLine();
  cout<<" : "<<fgJob->getPID()<<endl;
  waitpid(fgJob->getPID(),NULL,WUNTRACED);
  if(smash.getMostRecentCommand() == fgJob->getCmd()){
    smash.SetMostRecentCommand(nullptr);
  }
}
void BackgroundCommand::execute(){ //done
  JobsList::JobEntry* bgJob;
  if(numOfArgs==1){
    bgJob = SmallShell::getInstance().getLastStoppedJob();
    if(bgJob == nullptr){
      cerr<<"smash error: bg: there is no stopped jobs to resume"<<endl;
      return;
    }
  }
  else if(numOfArgs==2){
    try{
      bgJob = SmallShell::getInstance().getJobByID(stoi(argv[1]));
    }
    catch(...){
      cerr << "smash error: kill: invalid arguments" << endl;
      return;
    }  
    if(bgJob == nullptr){
      cerr << "smash error: bg: job-id " << argv[1] << " does not exist" << endl;
      return;
    }
    if(!bgJob->isStopped()){
      cerr << "smash error: bg: job-id " << argv[1] << " is already running in the background" << endl;
      return;
    }
  }
  else{
    cerr<<"smash error: bg: invalid arguments"<<endl;
    return;
  }
  bgJob->printOriginalLine();
  cout<<" : "<<bgJob->getPID()<<endl;
  kill(bgJob->getPID(), SIGCONT);//sends the signal
  bgJob->continueJob();
}
void JobsCommand::execute(){ 
  SmallShell::getInstance().printJobs();
}
void QuitCommand::execute(){
  if(numOfArgs != 2 || strcmp(argv[1], "kill")) exit(1); 
  else{
    SmallShell::getInstance().quitCmd();
    exit(1);
  }
}
void KillCommand::execute(){
  if(numOfArgs!=3){
    cerr<<"smash error: kill: invalid arguments"<<endl;
    return;
  }
  if(argv[1][0]!='-'){
    cerr<<"smash error: kill: invalid arguments"<<endl;
    return;
  }
  int sigNum, jobId;
  try{
    sigNum = (-1) * stoi(string(argv[1]));
    jobId = stoi(string(argv[2]));
  }catch(...){
    cerr<<"smash error: kill: invalid arguments"<<endl;
    return;
  }
  if(sigNum < 0 || sigNum > 31){
    cerr<<"smash error: kill: invalid arguments"<<endl;
    return;
  }
  JobsList::JobEntry* jobPtr = SmallShell::getInstance().getJobByID(jobId);
  if(jobPtr == nullptr){
    cerr<<"smash error: kill: job-id "<<argv[2]<<" does not exist"<<endl;
    return;
  }
  if(kill(jobPtr->getPID(), sigNum) == -1)
    perror("smash error: kill failed");
  else{
    if (sigNum == SIGSTOP) jobPtr->stop();
    if (sigNum == SIGCONT) jobPtr->continueJob();
    cout<<"signal number "<<sigNum<<" was sent to pid "<<jobPtr->getPID()<<endl; 
  }
}

void ChmodCommand::execute(){
  if(numOfArgs != 3)
    cerr<<"smash error: chmod: invalid arguments"<<endl;
  try{
    if(chmod(argv[2],stoi(argv[1],nullptr,8)) == -1){
      cerr<<"smash error: chmod: invalid arguments"<<endl;
    }
  }
  catch(...){
    cerr<<"smash error: chmod: invalid arguments"<<endl;
  }
}

void GetFileTypeCommand::execute(){
  if(numOfArgs!=2){
    cerr<<"smash error: gettype: invalid arguments"<<endl;
    return;
  }
  struct stat fileInfo;
  if(stat(argv[1],&fileInfo) == -1){
    perror("syscall failed\n");
    return;
  }
  cout << argv[1] << "'s type is \"";
  mode_t temp = fileInfo.st_mode & S_IFMT;
  if(temp == S_IFREG) cout<<"regular file";
  else if(temp == S_IFDIR) cout<<"directory";
  else if(temp == S_IFCHR) cout<<"character device";
  else if(temp == S_IFBLK) cout<<"block device";
  else if(temp == S_IFIFO) cout<<"FIFO";
  else if(temp == S_IFLNK) cout<<"symobic link";
  else if(temp == S_IFSOCK) cout<<"socket";

  cout<<"\" and takes up " << fileInfo.st_size << " bytes" << endl;
  
}

void SetcoreCommand::execute(){
  if(numOfArgs != 3){
    cerr<<"smash error: setcore: invalid arguments"<<endl;
    return;
  }
  int jobid,coreNum;
  try{
    jobid = stoi(argv[1]);
    coreNum = stoi(argv[2]);
  }
  catch(...){
      cerr<<"smash error: setcore: invalid arguments"<<endl;
      return;
  }
  if (coreNum>=sysconf(_SC_NPROCESSORS_CONF)||coreNum<0)
  {
    cerr<<"smash error: setcore: invalid core number"<<endl;
    return;
  }
  JobsList::JobEntry * jobPtr = SmallShell::getInstance().getJobByID(jobid);
  if(jobPtr == nullptr){
    cerr<<"smash error: setcore: job-id "<<argv[1]<<" does not exist\n";
    return;
  }
  int pid = jobPtr->getPID();
  cpu_set_t cpu;
  CPU_SET(coreNum,&cpu);
  if(sched_setaffinity(pid, sizeof(cpu_set_t),&cpu)==-1){
      cerr<<"smash error: setcore: invalid core number"<<endl;
  }
}

void RedirectionCommand::execute(){
  int originalFd = dup(fileno(stdout));
  int fd;
  if(appendFlag){
    fd = open(fileName.c_str() ,O_WRONLY | O_APPEND | O_CREAT, 0655);
    if(fd == -1){
      perror("smash error: open failed");
      return;
    }
    freopen(fileName.c_str(), "a", stdout);
  }
  else{
    fd = open(fileName.c_str() ,O_WRONLY | O_TRUNC | O_CREAT, 0655);
    if(fd == -1){
      perror("smash error: open failed");
      return;
    }
    freopen(fileName.c_str(), "w", stdout);
  }
  command->execute();
  dup2(originalFd, fileno(stdout));
  close(fd);
}

void PipeCommand::execute(){
  int my_pipe[2];
  if(pipe(my_pipe) == -1){
    cerr<<"smash error: pipe failed"<<endl;
    return;
  }
  int pid2;
  int pid = fork();
  if(pid == -1){
    cerr<<"smash error: fork failed"<<endl;
    return;
  }
  else if (pid == 0) { // son
    setpgrp();
    if(dup2(my_pipe[1], 1 + (int)errorFlag) == -1){
      cerr<<"smash error: dup2 failed"<<endl;
      return;
    }
    if(close(my_pipe[0]) == -1){
      cerr<<"smash error: close failed"<<endl;
      return;
    }
    command1Ptr->execute();
    exit(0);
  }
  else { // father
    pid2 = fork();
    if(pid2 == -1){
      cerr<<"smash error: fork failed"<<endl;
      return;
    }
    else if(pid2 == 0){
      setpgrp();
      if(dup2(my_pipe[0], 0) == -1){
      cerr<<"smash error: dup2 failed"<<endl;
      return;
      }
      if(close(my_pipe[1]) == -1){
        cerr<<"smash error: close failed"<<endl;
        return;
      }
      command2Ptr->execute();
      exit(0);
    }
  }
  if(close(my_pipe[0]) == -1){
    cerr<<"smash error: close failed"<<endl;
    return;
  }
  if(close(my_pipe[1]) == -1){
    cerr<<"smash error: close failed"<<endl;
    return;
  }
  waitpid(pid, NULL, WUNTRACED);
  waitpid(pid2, NULL, WUNTRACED);
}

void ExternalCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  if(!backGroundFlag) smash.SetMostRecentCommand(this);
  int pid = fork();
  if(pid == -1){
    cerr<<"fork failed"<<endl;
  }
  else if(pid == 0){
    setpgrp();
    size_t position = original.find("*?",0,1);
    if(position != string::npos){
       char* const argsForBash[]= {strdup("/bin/bash"), strdup("-c"), (char*)(_trim(original).c_str()), NULL};
      if(execvp(argsForBash[0],argsForBash) == -1){
        perror("smash error: execvp failed");
        return;
      }
    }
    else{
      if(execv(argv[0], argv) == -1 ){
        strcpy(argv[0], (string("/bin/") + string(argv[0])).c_str());
        if(execv(argv[0], argv) == -1 ){
          strcpy(argv[0], (string("/usr") + string(argv[0])).c_str());
          if(execv(argv[0], argv) == -1 ){
            perror("smash error: execv failed");
            exit(1);
          }
        }
      }
      /*if(argv[0][0] != '.'){
        strcpy(argv[0], (string("/bin/") + string(argv[0])).c_str());
      }
      if(execv(argv[0], argv) == -1){
        perror("smash error: execv failed");
        exit(1);
      } */ 
    }
  }
  else{
    smash.addJob(this, pid);
    if(!backGroundFlag){
      if(waitpid(pid, NULL, WUNTRACED) == -1){
        perror("smash error: waitpid failed");
        return;
      }
    }
  }
  if(smash.getMostRecentCommand() == this){
    smash.SetMostRecentCommand(nullptr);
  }
  return;
}

void ExternalCommand::printName() const{
  cout<<original;
}

void ExternalCommand::printOriginal(){
  cout<<original;
}

//---------------------time out-----------------------------------//

TimeoutCommand::TimeoutCommand(const char* cmd_line, const string& og, bool bg) :
          BuiltInCommand(cmd_line),original(og), bgFlag(bg){}

void TimeoutCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  int seconds;
  if(numOfArgs < 3){
    cerr << "smash error: timeout: invalid arguments" << endl;
    return;
  }
  try{
    seconds = stoi(argv[1]);
  }
  catch(...){
    cerr << "smash error: timeout: invalid arguments" << endl;
    return;
  }
  if(seconds <= 0){
    cerr << "smash error: timeout: invalid arguments" << endl;
    return;
  }
  int pid1 = fork();
  if (pid1 == 0){ // this process pid is pid1
    int pid2 = fork();
    if(pid2 == 0){ //this process pid is pid2
      Command* cmdPtr = smash.CreateCommand(string(original).substr(9 + floor(log10(seconds))).c_str());
      cmdPtr->execute();
      exit(0);
    }
    else{ //this is still pid1
      sleep(seconds);
      if(waitpid(pid2, NULL, WNOHANG) == -1)
        exit(0);
      if(kill(pid2, SIGKILL) == -1)
        exit(0);
      cout << original << endl;
      cout << "smash: got an alarm" << endl;
      cout << "smash: " << original << " timed out!" << endl;
      exit(0);
    }
  }
  else{
    if(!bgFlag){
      waitpid(pid1, NULL, WUNTRACED);
    }
  }
}