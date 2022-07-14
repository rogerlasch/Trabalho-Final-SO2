
#if defined(_WIN32)
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#endif
#include <exception>
#include <stdexcept>
#include "../dependencies/so2_includes.h"
#include "basic_connection.h"
#include "ClienteSocket.h"

/* Estrutura responsável para iniciar as bibliotecas de rede do Windows e garantir
 o correto descarregamento quando o programa e encerrado */
struct WinSockCleaner
{
    WinSockCleaner()
    {
        WSADATA wsadata;
        int32 res = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if (res != 0)
        {
            throw std::runtime_error("Erro ao inicializar a winsock. Programa n�o pode continuar!");
        }
    }
    ~WinSockCleaner()
    {
        WSACleanup();
    }
};

WinSockCleaner cls;

// Tenta abrir uma conexao com o servidor no endereço/porta especificado 
shared_connection s_connect(const std::string ip_address, uint32 port)
{
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int32 res = getaddrinfo(ip_address.c_str(), std::to_string(port).c_str(), &hints, &result);
    
    if (res != 0)
    {
        return shared_connection();
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sock == INVALID_SOCKET)
        {
            return shared_connection();
        }

        res = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        
        if (res == SOCKET_ERROR)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET)
    {
        return shared_connection();
    }

    u_long x = 1;
    ioctlsocket(sock, FIONBIO, &x);
    shared_connection c = std::make_shared<basic_connection>();
    c->setSock(sock);
    c->setConState(con_connected);

    return c;
}

// Recebe os dados do servidor 
std::string s_request(shared_connection &c)
{
    std::string str = "";
    str.resize(1024);
    int32 res = recv(c->getSock(), &str[0], str.size(), 0);
    int32 error = WSAGetLastError();

    if ((res == 0) || (error == WSAENOTCONN) || (error == WSAESHUTDOWN))
    {
        c->setConState(con_disconnected);
        return "";
    }

    else if (res == SOCKET_ERROR)
    {
        return "";
    }

    if (res >= 0)
    {
        str.resize(res);
    }

    return str;
}

// Envia dados para o servidor
void s_send(shared_connection &c)
{
    if ((c == NULL) || (c->getConState() != con_connected))
    {
        return;
    }

    std::string line = c->get_line_to_send();

    if (line.size() == 0)
    {
        return;
    }

    line += '\n';
    int32 res = send(c->getSock(), &line[0], line.size(), 0);
    
    if (res == SOCKET_ERROR)
    {
        _log("Erro!");
        res = WSAGetLastError();

        if ((res == WSAENETDOWN) || (res == WSAENOTCONN) || (res == WSAESHUTDOWN))
        {
            c->setConState(con_disconnected);
        }
    }
}

// Fecha a conexao com o servidor
void s_disconnect(shared_connection &c)
{
    if ((c == NULL) || (c->getConState() != con_connected))
    {
        return;
    }
    
    shutdown(c->getSock(), SD_SEND);
    closesocket(c->getSock());
    c->setSock(0);
    c->setConState(con_disconnected);
}
