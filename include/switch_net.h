#ifndef SWITCH_NET_H
#define SWITCH_NET_H

/* The console's BSD sockets service, for the network race. It is started the first time the network
   is used (not at start-up, so a game that never goes on the network never touches it) and closed
   again when the program ends, after the IPX client has stopped: a program that leaves the service
   open when it exits can bring the system's socket module down, and the Homebrew Menu with it. */

int SwitchNet_start(void);      /* 1 when sockets are available */

#endif // SWITCH_NET_H
