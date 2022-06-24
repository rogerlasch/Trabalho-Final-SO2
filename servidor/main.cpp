
#pragma comment(lib, "../dependencies/libs/biblioteca.lib")
#include <signal.h>
#include<cassert>
#include<map>
#include<memory>
#include<vector>
#include"../dependencies/so2_includes.h"
#include"basic_connection.h"
#include"socket_manipulation.h"
#include"Card.h"
#include"main.h"



using namespace std;
using namespace dlb;

using namespace dlb;
using namespace std;


uint32 DefaultPort=4000;
uint32 ServerState;
shared_mutex  mtx_server;
void signal_shutdown(int32 x);
void processEvent(dlb_event* ev);

int main()
{
try {
setlocale(LC_ALL, "Portuguese");
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
s_setState(server_running);
_log("Servidor iniciado na porta {}", DefaultPort);
_log("Para interromper, tecle CTRL+c");
bool stop=false;
shared_card c=make_shared<Card>();
c->setType(plus_four);
c->setColor(red);
c->setNumber(5);
_log("{}", c->toString());
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
s_send_to_all("Aviso... O servidor estï¿½ sendo parado...");
s_setState(server_shuting_down);
}

void processEvent(dlb_event* ev)
{
switch(ev->type)
{
case event_connect:
{
shared_connection c=make_shared<basic_connection>();
c->setSock(ev->id);
c->setConState(con_connected);
s_insert_connection(c);
string str=fmt::format("Alguém com socket {} se conectou...", ev->id);
s_send_to_all(str);
_log(str);
break;
}
case event_disconnect:
{
shared_connection c=s_find_connection(ev->id);
if(c==NULL)
{
return;
}
s_disconnect_sock(ev->id);
string str=fmt::format("Alguém com socket {} se desconectou...", ev->id);
s_send_to_all(str);
_log(str);
break;
}
case event_receive:
{
shared_connection c=s_find_connection(ev->id);
_log("Mensagem recebida de {}: \"{}\"", c->getSock(), ev->data);
c->print(ev->data);
if(ev->data=="quit")
{
c->print("Tchau!");
s_disconnect_sock(ev->id);
}
break;
}
}
}
