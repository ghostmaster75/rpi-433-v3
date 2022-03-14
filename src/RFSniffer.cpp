/*
  RFSniffer
  Usage: ./RFSniffer [<pulseLength>, <pin>] 
  [] = optional
  Hacked from http://code.google.com/p/rc-switch/
  by @justy to provide a handy RF code sniffer
*/
#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex g_mutex;
condition_variable g_cv;
RCSwitch mySwitch;

int pin = 2;
bool g_ready = false;

void received() {
	while (true) {
		unique_lock<mutex> ul(g_mutex);
		g_cv.wait(ul, []() { return g_ready; });
		int value = mySwitch.getReceivedValue();
        	if (value == 0) {
	        	printf("Unknown encoding\n");
        	} else {
			printf("{\"code\": %i, \"pulseLength\": %i}", mySwitch.getReceivedValue(), mySwitch.getReceivedDelay());
		}
		fflush(stdout);
		mySwitch.resetAvailable();
		g_ready = false;
		ul.unlock();
		g_cv.notify_one();
	}
}

void receiving() {
	while (true) {
		unique_lock<mutex> ul(g_mutex);
		if (mySwitch.available()) {
			g_ready = true;
			ul.unlock();
			g_cv.notify_one();
			ul.lock();
		}
		auto now = chrono::system_clock::now();
		g_cv.wait(ul, []() { return g_ready == false; });
		this_thread::sleep_for (chrono::milliseconds(100));
	}
}

void receivedThread(int n) { received(); }

void receivingThread(int n) { receiving(); }

int main(int argc, char *argv[]) {
	if(wiringPiSetup() == -1) {
		printf("wiringPiSetup failed, exiting...");
		return 0;
	}
	int pulseLength = 0;
	if (argv[1] != NULL) pulseLength = atoi(argv[1]);
	if (argv[2] != NULL) pin = atoi(argv[2]);

	mySwitch = RCSwitch();

	if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
	mySwitch.enableReceive(pin);  // Receiver on interrupt 0 => that is pin #2

	int times = 100;
	thread t1(receivedThread, times);
	thread t2(receivingThread, times);
	t1.join();
	t2.join();

	exit(0);
}
