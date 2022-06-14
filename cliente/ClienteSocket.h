


#ifndef CLIENTESOCKET_H
#define CLIENTESOCKET_H

shared_connection s_connect(const std::string ip_address, uint32 port);
std::string s_request(shared_connection& c);
void s_send(shared_connection& c);
void s_disconnect(shared_connection& c);
#endif
