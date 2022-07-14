



#ifndef MAIN_H
#define MAIN_H

enum SERVER_STATES
{
server_starting=0, // Quando nao deve aceitar conexoes (esta se configurando ainda...)
server_running, // Recebe as conexoes, aceita, gerencia, da o loop nas mesas, faz os bots jogarem
server_shuting_down, // Nao aceita mais conexoes, libera todos os recursos alocados
server_waiting_exit // Aguarda o servidor ser fechado
};

void s_setState(uint32 st);
uint32 s_getState();

//As proximas funcoes sao comandos. Nao devem ser invocadas diretamente.
void do_help(shared_player& ch, const std::string& args);
void do_commands(shared_player& ch, const std::string& args);
void do_create_party(shared_player& ch, const std::string& args);
void do_chat(shared_player& ch, const std::string& args);
void do_quit(shared_player& ch, const std::string& args);
void do_who(shared_player& ch, const std::string& args);
void do_table(shared_player& ch, const std::string& args);
#endif
