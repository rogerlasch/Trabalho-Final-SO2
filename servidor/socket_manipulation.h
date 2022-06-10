


#ifndef SOCKET_MANIPULATION_H
#define SOCKET_MANIPULATION_H

#include<shared_mutex>
#include<mutex>
#include<string>
#include<sstream>

bool s_setup_server(uint32 port);
void s_shutdown_server();
bool server_is_running();
void s_sock_loop();
bool s_send(int32 sock, const std::string& data);
void s_send_to_all(const std::string& data);
void s_disconnect_sock(uint32 sock);
bool s_insert_connection(const shared_connection& c);
shared_connection s_find_connection(uint32 sock);
#endif
