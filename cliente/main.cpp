
#pragma comment(lib, "../../dependencies/libs/biblioteca.lib")
#include<iostream>
#include<thread>
#include<future>
#include"../dependencies/so2_includes.h"
#include"basic_connection.h"
#include"ClienteSocket.h"


using namespace std;
using namespace dlb;

string ip_address="localhost";
uint32 port=4000;
static shared_connection con;
void connection_loop();
int main()
{
setlocale(LC_ALL, "Portuguese");
system("chcp 1252");
con=s_connect(ip_address, port);
if(con==NULL)
{
_log("Erro ao se conectar no servidor {} na porta {}.", ip_address, port);
return 0;
}
thread th(connection_loop);
while(con->getConState()==con_connected)
{
string line="";
cin.clear();
cin.sync();
//cin.ignore();
getline(cin, line);
if(line.size()>0)
{
    con->print(line);
}
if(line=="quit")
{
s_disconnect(con);
}
}
th.join();
return 0;
}

void connection_loop()
{
FuncTimer ts(__FUNCTION__);
    while(con->getConState()==con_connected)
    {
        this_thread::sleep_for(chrono::milliseconds(20));
        string data=s_request(con);
        s_send(con);
if(data.size()>0)
{
_log(data);
}
        }
    }
