
#pragma comment(lib, "..\\..\\dependencies\\libs\\biblioteca.lib")
#include <signal.h>
#include<cassert>
#include<random>
#include<algorithm>
#include<map>
#include<functional>
#include<memory>
#include<vector>
#include"../dependencies/so2_includes.h"
#include"StringUtils.h"
#include"basic_connection.h"
#include"socket_manipulation.h"
#include"Card.h"
#include"Player.h"
#include"Table.h"
#include"main.h"

using namespace dlb;
using namespace std;

uint32 DefaultPort=4000;
uint32 ServerState=0;
shared_mutex  mtx_server;
typedef function<void(shared_player&, const string&)> command_function;
map<string, command_function> cmd_table;
map<uint32, shared_table> tables;
void signal_shutdown(int32 x);
void s_setState(uint32 st);
uint32 s_getState();
void load_commands();
void processEvent(dlb_event* ev);

int main()
{
try {
setlocale(LC_ALL, "Portuguese");
system("chcp 1252");
signal(SIGTERM, signal_shutdown);
signal(SIGINT, signal_shutdown);
signal(SIGABRT, signal_shutdown);
s_setState(server_starting);
_log("Inicializando servidor...");
if(!s_setup_server(DefaultPort))
{
return 0;
}
uint32 res=dlb_worker_create(2, 0, processEvent);
_log("Total de threads de apoio criados: {}", res);
load_commands();
s_setState(server_running);
_log("Servidor iniciado na porta {}", DefaultPort);
_log("Para interromper, tecle CTRL+c");
bool stop=false;
while(stop==false)
{
this_thread::sleep_for(chrono::milliseconds(5));
switch(s_getState())
{
case server_running:
{
s_sock_loop();
break;
}
default:
{
stop=true;
}
}
}
s_shutdown_server();
_log("Total de threads parados: {}", dlb_worker_stop_all());
dlb_event_cleanup();
s_setState(server_waiting_exit);
_log("Servidor finalizado!");
} catch(const exception& e) {
_log("Exception! {}", e.what());
}
return 0;
}

void load_commands()
{
cmd_table.clear();
cmd_table={
{"ajuda", do_help},
{"comandos", do_commands},
{"criar_partida", do_create_party},
{"chat", do_chat},
{"mesa", do_table},
{"sair", do_quit},
{"who", do_who},
};
}

void s_setState(uint32 st)
{
unique_lock<shared_mutex> lck(mtx_server);
ServerState=st;
}

uint32 s_getState()
{
shared_lock<shared_mutex> lck(mtx_server);
return ServerState;
}

void signal_shutdown(int x)
{
s_send_to_all("Aviso... O servidor está sendo parado...");
s_setState(server_shuting_down);
}

void processEvent(dlb_event* ev)
{
static StringUtils cs;
switch(ev->type)
{
case 150:
{
auto it=tables.find(ev->id);
if(it!=tables.end())
{
it->second->interact_bot();
}
break;
}
case event_connect:
{
shared_player ch=make_shared<Player>();
ch->setSock(ev->id);
ch->setConState(con_connected);
s_insert_connection(ch);
ch->print("Saldações... Para começar, digite o seu nome de usuário com no mínimo 4 caracteres.");
break;
}
case event_disconnect:
{
shared_player ch=dynamic_pointer_cast<Player>(s_find_connection(ev->id));
if(ch==NULL)
{
return;
}
s_disconnect_sock(ev->id);
if(ch->getName().size()>0)
{
_s_send_to_all("{} nos deixou!", ch->getName());
}
break;
}
//Dados chegaram do jogador...
case event_receive:
{
shared_player ch=dynamic_pointer_cast<Player>(s_find_connection(ev->id));
_log("Mensagem recebida de {}: \"{}\"", ch->getSock(), ev->data);
if(ev->data=="sair")
{
do_quit(ch, "");
return;
}
//Primeiro, determinar em que estágio o jogador está...
if(ch->getName().size()==0)
{
if(ev->data.size()<4)
{
ch->print("Digite um nome com pelo menos 4 caracteres.");
return;
}
ch->setName(ev->data);
ch->print(fmt::format("Bem-vindo ao jogo, {}!", ch->getName()));
ch->print("Digite comandos para ver uma lista de comandos disponíveis.");
_s_send_to_all("{} se conectou!", ch->getName());
}
//Está em uma mesa já? Isso pode significar que ele está jogando.
else if(ch->getTable()!=NULL)
{
shared_table tl=ch->getTable();
tl->process_command(ch, ev->data);
}
//Está só matando tempo, não sabe o que quer ainda...
else
{
//Verifique se o jogador digitou algum comando, separa o comando dos argumentos...
string cmd="", args="";
cs.parse(ev->data, cmd, args);
//Pesquisa na tabela de comandos para determinar se ele digitou algum comando válido.
auto it=cmd_table.find(cmd);
//Caso nenhum comando tenha sido encontrado...
if(it==cmd_table.end())
{
ch->print("O que?");
return;
}
//Chame o comando que o jogador digitou passando os argumentos...
it->second(ch, args);
}
break;
}
}
}

//Comandos em geral que o jogador pode digitar...
void do_help(shared_player& ch, const std::string& args)
{
ch->print("Você pode digitar o nome do comando seguido por \'?\' para obter mais informações sobre ele.");
ch->print("Digite comandos para ver uma lista de todos os comandos disponíveis.");
}

void do_commands(shared_player& ch, const std::string& args)
{
stringstream ss;
ss<<"Lista de comandos disponíveis:"<<endl;
for(auto it=cmd_table.begin(); it!=cmd_table.end(); ++it)
{
ss<<it->first<<endl;
}
ss<<cmd_table.size()<<" comandos encontrados."<<endl;
ss<<"Digite o comando seguido de \'?\' para mais detalhes."<<endl;
ch->print(ss.str());
}

void do_create_party(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando criar_partida irá criar uma nova mesa para aguardar jogadores.");
return;
}
shared_table ts=make_shared<Table>();
ts->add_player(ch);
tables.insert(make_pair(ts->getId(), ts));
}

void do_chat(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando chat lhe permite enviar umamensagem pública para todo mundo que estiver conectado, exceto quem estiver jogando.");
ch->print("Uso: \'chat Menssagem\'");
return;
}
string msg=fmt::format("{} disse \'{}\'", ch->getName(), args);
connection_list ls=get_connections();
for(auto it=ls.begin(); it!=ls.end(); ++it)
{
shared_player ch2=dynamic_pointer_cast<Player>(it->second);
if((ch2!=NULL)&&(ch2->getName().size()>0)&&(ch2->getTable()==NULL))
{
ch2->print(msg);
}
}
}

void do_quit(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando sair irá lhe desconectar do servidor.");
return;
}
ch->print("Até mais!");
s_disconnect_sock(ch->getSock());
}

void do_who(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando who mostra todo mundo que está online no momento.");
ch->print("Uso: \'who\'");
return;
}
stringstream ss;
ss<<"Pessoas online no momento:"<<endl;
int32 x=0;
connection_list ls=get_connections();
for(auto it=ls.begin(); it!=ls.end(); ++it)
{
shared_player ch2=dynamic_pointer_cast<Player>(it->second);
if((ch2!=NULL)&&(ch2->getName().size()>0))
{
ss<<ch2->getName()<<endl;
x++;
}
}
if(x>0)
{
ss<<x<<" pessoas encontradas."<<endl;
}
ch->print(ss.str());
}

void do_table(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando mesa tem duas finalidades.");
ch->print("Digitando \"mesa\" sem argumentos irá mostrar todas as mesas de jogo atualmente criadas.");
ch->print("Digitando \"mesa <Número>\" fará que você entre em uma mesa como expectador.\nCaso a mesa ainda não esteja com um jogo ativo, isto lhe tornará um jogador a sim que o jogo for iniciado.");
return;
}
if(args.size()==0)
{
stringstream ss;
ss<<"Mesas disponíveis:"<<endl;
for(auto it=tables.begin(); it!=tables.end(); ++it)
{
ss<<it->second->toString()<<endl;
}
ss<<"Total de mesas encontradas: "<<tables.size()<<endl;
ch->print(ss.str());
return;
}
uint32 x=atoi(args.c_str());
auto it=tables.find(x);
if(it==tables.end())
{
ch->print(fmt::format("O argumento {} é inválido!", args));
return;
}
it->second->add_player(ch);
}
