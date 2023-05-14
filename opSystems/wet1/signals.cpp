#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

void ctrlZHandler(int sig_num) {
	cout<<"smash: got ctrl-Z"<<endl;
  int pid = SmallShell::getInstance().getLastJobsPID();
  kill(pid,SIGSTOP);
  //SmallShell::getInstance().addJob(SmallShell::getInstance().getMostRecentCommand(), pid);
  cout<<"smash: process "<< pid <<" was stopped"<<endl;
}

void ctrlCHandler(int sig_num) {
  int pid = SmallShell::getInstance().getLastJobsPID();
  cout<<"smash: got ctrl-C"<<endl;
  kill(pid,SIGKILL);
  cout<<"smash: process "<<pid<<" was killed"<<endl;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}
