



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
//As próximas funções são comandos. Não devem ser invocadas diretamente.
void do_help(shared_player& ch, const std::string& args);
void do_commands(shared_player& ch, const std::string& args);
void do_create_party(shared_player& ch, const std::string& args);
void do_chat(shared_player& ch, const std::string& args);
void do_quit(shared_player& ch, const std::string& args);
void do_who(shared_player& ch, const std::string& args);
void do_table(shared_player& ch, const std::string& args);
#endif
