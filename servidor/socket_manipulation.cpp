
#if defined(_WIN32)
#define NOMINMAX
#include<winsock2.h>
#include<ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#endif
#include<unordered_map>
#include<memory>
#include<string>
#include"..\dependencies\so2_includes.h"
#include"basic_connection.h"
#include"socket_manipulation.h"


using namespace dlb;

static bool Sstarted=false;
SOCKET mainsock=0;
static std::unordered_map<uint32, shared_connection> connections;

bool s_setup_server()
{
if(server_is_running()==true)
{
return false;
}
WSADATA wsadata;
int32 res=WSAStartup(MAKEWORD(2,2), &wsadata);
if(res!=0)
{
_log("Erro ao iniciar a winsock. Erro: {}", res);
return false;
}
struct addrinfo* result=NULL, *ptr=NULL, hints;
ZeroMemory(&hints, sizeof (hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
hints.ai_flags = AI_PASSIVE;
res=getaddrinfo(NULL, "4000", &hints, &result);
if (res!=0)
{
_log("Getaddrinfo falhou! C�digo: {}", res);
s_shutdown_server();
return false;
}
mainsock=socket(result->ai_family, result->ai_socktype, result->ai_protocol);
if(mainsock==INVALID_SOCKET)
{
_log("Erro ao criar o socket. Erro: {}", WSAGetLastError());
s_shutdown_server();
freeaddrinfo(result);
return false;
}
res=bind(mainsock, result->ai_addr, (int)result->ai_addrlen);
if(res==SOCKET_ERROR)
{
_log("Bind falhou com o c�digo: {}", WSAGetLastError());
freeaddrinfo(result);
s_shutdown_server();
return false;
    }
freeaddrinfo(result);
if(listen(mainsock, SOMAXCONN)==SOCKET_ERROR)
{
_log("Fun��o listen falhou com o c�digo: {}", WSAGetLastError());
s_shutdown_server();
return false;
}
u_long x=1;
res=ioctlsocket(mainsock, FIONBIO, &x);
if(res!=0)
{
_log("Erro ao tornar o socket as�ncrono. C�digo: {}", WSAGetLastError());
s_shutdown_server();
return false;
}
Sstarted=true;
return true;
}

void s_shutdown_server()
{
if(!server_is_running())
{
return;
}
if(mainsock!=0)
{
closesocket(mainsock);
}
mainsock=0;
WSACleanup();
mainsock=0;
Sstarted=false;
}

bool server_is_running()
{
return Sstarted;
}

//Deve ser chamada continuamente para enviar e receber dados, bem como processar a entrada do usu�rio...
void s_sock_loop()
{
//Verificar se o servidor est� ativo e ouvindo...
if((!server_is_running())||(mainsock==0)||(mainsock==SOCKET_ERROR))
{
return;
}
//Declarar extruturas de sele��o e etc...
FD_SET reads;
FD_SET writes;
FD_ZERO(&reads);
FD_ZERO(&writes);
FD_SET(mainsock, &reads);
//Adicionar todos sockets para as verifica��es...
if(connections.size()>0)
{
for(auto it=connections.begin(); it!=connections.end(); ++it)
{
FD_SET(it->first, &reads);
FD_SET(it->first, &writes);
}
}
struct timeval tv;
tv.tv_sec=0;
tv.tv_usec=(5000*1000);
int32 res=select(0, &reads, &writes, NULL, &tv);
if(res==SOCKET_ERROR)
{
FD_ZERO(&reads);
FD_ZERO(&writes);
return;
}
//Verificar se o socket mestre recebeu algo...
if(FD_ISSET(mainsock, &reads))
{
//Tentar aceitar a nova conex�o...
SOCKET s=accept(mainsock, NULL, NULL);
if(s==SOCKET_ERROR)
{
_log("Erro ao aceitar conex�es... {}", WSAGetLastError());
}
else
{
//Definir para o modo sem bloqueio...
u_long x=1;
ioctlsocket(s, FIONBIO, &x);
//Envia o socket por evento para que seja atribu�do um objeto de conex�o&player.
dlb_event_send(s, event_connect);
}
}
//Agora que o pior j� foi, vamos iterar pela lista interna de sockets e verificar se algo deve ser feito...
if(connections.size()>0)
{
for(auto it=connections.begin(); it!=connections.end(); ++it)
{
//Verificar se algu�m enviou dados para n�s...
if(FD_ISSET(it->first, &reads))
{
//Ler os dados...
std::string s;
s.resize(1024);
res=recv(it->first, &s[0], s.size(), 0);
if(res==SOCKET_ERROR)
{
closesocket(it->first);
//Envia um evento de desconex�o...
dlb_event_send(it->first, event_disconnect);
continue;
}
else if(res==0)//A conex�o foi fechada...
{
dlb_event_send(it->first, event_disconnect);
continue;
}
s.resize(res);
it->second->append_string_input(s);
it->second->process_input();
}
//Vamos verificar se podemos enviar mais dados...
if(FD_ISSET(it->first, &writes))
{
//Envia dados, se poss�vel...
std::string line=it->second->get_line_to_send();
//Se nada para enviar...
if(line.size()==0)
{
continue;
}
line+='\n';
res=send(it->first, &line[0], line.size(), 0);
if(res==SOCKET_ERROR)
{
closesocket(it->first);
dlb_event_send(it->first, event_disconnect);
continue;
}
}
}
}
}

//Enfileira uma string para envio futuro pelo loop...
bool s_send(int32 sock, const std::string& data)
{
if((Sstarted==false)||(data.size()==0))
{
return false;
}
auto it=connections.find(sock);
if(it==connections.end())
{
return false;
}
it->second->print(data);
return true;
}

void s_send_to_all(const std::string& data)
{
for(auto it=connections.begin(); it!=connections.end(); ++it)
{
it->second->print(data);
}
}

void s_disconnect(int32 sock)
{
if(connections.count(sock)==0)
{
return;
}
int32 res=shutdown(sock, SD_SEND);
if(res==SOCKET_ERROR)
{
    closesocket(sock);
connections.at(sock)->setSock(0);
}
}

bool s_insert_connection(const shared_connection& c)
{
if(connections.count(c->getSock())>0)
{
return false;
}
connections.insert(make_pair(c->getSock(), c));
return true;
}

shared_connection s_find_connection(uint32 sock)
{
auto it=connections.find(sock);
return ((it==connections.end()) ? shared_connection() : it->second);
}
