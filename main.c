/* ========================================================================= */
/* main.c -- rfcontrol entry point                                           */
/* ========================================================================= */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "UDPSocketClient.h"

pid_t myPID = -1;

int main(int ac, char **av) {
myPID= getpid();
UDPSocketClient();

return 0;
}
