

#ifndef BASIC_CONNECTION_H
#define BASIC_CONNECTION_H

#include <unordered_map>
#include <vector>
#include <string>

// Estados de conexão
enum connection_stats
{
    con_connected = 1,
    con_disconnected
};

// Eventos que dizem o que aconteceu na conexão
enum NetEvents
{
    event_connect = 1, // Alguem se conectou, reserve recursos para ele...
    event_receive,     // O peer enviou uma mensagem...
    event_disconnect,  // O peer se desconectou ou perdeu a conexao
};


class basic_connection
{
private:
    int32 sock;  // Socket
    uint32 conState; // Estado da conexão
    std::string cmd_line; // Linha processada
    std::vector<std::string> input_buffer; // Todas os comandos que foram recebidos e que devem ser processados
    std::vector<std::string> output_buffer; // Enfileira todas as mensagens que devem ser enviadas para o servidor ou cliente
    mutable std::shared_mutex mtx_input, mtx_output, mtx_con; // Sincronizam os acessos as propriedades da conexão

public:
    basic_connection();
    basic_connection(const basic_connection &bs) = delete;
    basic_connection &operator=(const basic_connection &bs) = delete;
    virtual ~basic_connection();
    virtual void print(const std::string &str);
    virtual std::string get_line_to_send();
    virtual void append_string_input(const std::string &str);
    virtual void process_input();
    void setSock(int32 sock);
    int32 getSock() const;
    void setConState(uint32 conState);
    uint32 getConState() const;
    bool is_connected() const;
};
typedef std::shared_ptr<basic_connection> shared_connection;
typedef std::unordered_map<int32, shared_connection> connection_list;

#endif
