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
  cout<<"smash: process "<< getpid() <<" was stopped"<<endl;
}

// void ctrlCHandler(int sig_num) {
//   cout<<"smash: got ctrl-C"<<endl;
//   //kill(getpid(),SIGKILL);
//   cout<<"smash: process "<<getpid()<<" was killed"<<endl;
// }

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}
