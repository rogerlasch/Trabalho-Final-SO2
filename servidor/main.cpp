
#pragma comment(lib, "../dependencies/libs/biblioteca.lib")
#include"../dependencies/so2_includes.h"
#include"basic_connection.h"
#include"socket_manipulation.h"


using namespace std;
using namespace dlb;

void processEvent(dlb_event* ev);
using namespace dlb;
using namespace std;
int main()
{
setlocale(LC_ALL, "Portuguese");
_log("Iniciando servidor... {}", s_setup_server());
while(server_is_running())
{
this_thread::sleep_for(chrono::milliseconds(5));
s_sock_loop();
}
s_shutdown_server();
return 0;
}

void processEvent(dlb_event* ev)
{
_log("Processando evento...");
switch(ev->type)
{
case event_connect:
{
shared_connection c=make_shared<basic_connection>();
c->setSock(ev->id);
c->setConState(con_connected);
s_insert_connection(c);
s_send_to_all("Alguém se conectou...");
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
s_send_to_all("Alguém se desconectou...");
break;
}
case event_receive:
{
break;
}
}
}
