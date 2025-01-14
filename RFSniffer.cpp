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
#include <thread>
using namespace std;

RCSwitch mySwitch;
 
int main(int argc, char *argv[]) {
  
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.
    int pin = 2;
     
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
     
    
	while(1) {
		if (mySwitch.available()) {
			int value = mySwitch.getReceivedValue();
			
			if (value == 0) {
				printf("Unknown encoding\n");
			} else {
				printf("{\"code\": %i, \"pulseLength\": %i}", mySwitch.getReceivedValue(), mySwitch.getReceivedDelay());
			}
			fflush(stdout);
			mySwitch.resetAvailable();
		}
		this_thread::sleep_for (std::chrono::milliseconds(200));
	}
	
	exit(0);
}
