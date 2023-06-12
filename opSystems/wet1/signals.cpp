#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

void ctrlZHandler(int sig_num) {
  SmallShell& smash = SmallShell::getInstance();
	cout<<"smash: got ctrl-Z"<<endl;
  ExternalCommand* cmd = smash.getMostRecentCommand();
  if(cmd == nullptr) return;
  int pid = smash.getFgJob()->getPID();
  kill(pid,SIGSTOP);
  cout<<"smash: process "<< pid <<" was stopped"<<endl;
  smash.getFgJob()->stop();
  smash.SetMostRecentCommand(nullptr);
}

void ctrlCHandler(int sig_num) {
  SmallShell& smash = SmallShell::getInstance();
  cout<<"smash: got ctrl-C"<<endl;
  ExternalCommand* cmd = smash.getMostRecentCommand();
  if(cmd == nullptr) return;
  int pid = smash.getFgJob()->getPID();
  kill(pid,SIGKILL);
  cout<<"smash: process "<<pid<<" was killed"<<endl;
  smash.removeJobByID(pid);
  smash.SetMostRecentCommand(nullptr);
}

void alarmHandler(int sig_num) {
  cout<<"smash: got an alarm"<<endl;
}
