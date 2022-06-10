



#ifndef MAIN_H
#define MAIN_H

enum SERVER_STATES
{
server_starting=0,
server_running,
server_shuting_down,
server_waiting_exit
};

void s_setState(uint32 st);
uint32 s_getState();
#endif
