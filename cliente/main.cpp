
#pragma comment(lib, "../dependencies/libs/biblioteca.lib")
#include<windows.h>
#include"../dependencies/so2_includes.h"
#include"basic_connection.h"


using namespace std;
using namespace dlb;

using namespace dlb;
using namespace std;

string ip_address="localhost";
uint32 port=4000;
static shared_connection con;
void connection_loop();
bool s_connect(const string& ip, uint32 port);
int main()
{
setlocale(LC_ALL, "Portuguese");
if(!s_connect(ip_address, port))
{
_log("Erro ao se conectar no servidor {} na porta {}.", ip_address, port);
while(con->isConnected())
{
this_thread::sleep_for(chrono::milliseconds(5));
}
return 0;
}
return 0;
}

void connection_loop()
{
}

bool s_connect(const string& ip, uint32 port)
{
return false;
}
