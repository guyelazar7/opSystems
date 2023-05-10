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
  int pid = getpid();
  cout<<"smash: process "<< pid <<" was stopped"<<endl;
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

