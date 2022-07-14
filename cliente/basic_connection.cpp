

#include <unordered_map>//Equivalente a HashMap em java...
#include "../dependencies/so2_includes.h"
#include "basic_connection.h"

using namespace std;

// Consntrutor da classe basic_connection
basic_connection::basic_connection()
{
    sock = 0;
    conState = 0;
    cmd_line = "";
    input_buffer.clear();
    output_buffer.clear();
}

// Destruidor da classe
basic_connection::~basic_connection()
{
}

// Enfileira uma mensagem para ser enviada para o servidor
void basic_connection::print(const string &str)
{
    // Pega direito de escrita ja que algo vai ser modificado...
    unique_lock<shared_mutex> lck(this->mtx_output);
    output_buffer.push_back(str);
}

// Recupera uma linha para ser enviada quando tiver espaço disponivel no buffer de envio
string basic_connection::get_line_to_send()
{
    unique_lock<shared_mutex> lck(this->mtx_output);
    string str = "";
    if (output_buffer.size() > 0)
    {
        str = output_buffer[0];
        output_buffer.erase(output_buffer.begin());
    }
    return str;
}

// Processa os dados recebidos, verificando se é possível processar um comando
//Um comando é enviado para o processamento quando um caractere de nova linha (\n) for encontrado.
//Se não, fique armazenando tudo na mesma linha até as condições serem atendidas.
void basic_connection::append_string_input(const string &str)
{
    unique_lock<shared_mutex> lck(this->mtx_input);
    for (uint32 i = 0; i < str.size(); i++)
    {
        if ((str[i] == '\r') || (str[i] == '\n'))
        {
            if (cmd_line.size() > 0)
            {
                input_buffer.push_back(cmd_line);
                cmd_line.resize(0);
            }
        }
        else
        {
            cmd_line += str[i];
        }
    }
}

// Pega todos os comandos enfileirados e despacha para a fila de eventos para ser processado por outros threads
//Isto no momento é usado apenas no servidor.
void basic_connection::process_input()
{
    unique_lock<shared_mutex> lck(this->mtx_input);
    if (input_buffer.size() > 0)
    {
        while (input_buffer.size() > 0)
        {
            // Despacha um evento para os workers darem um jeito...
            input_buffer.erase(input_buffer.begin());
        }
    }
}

// Atribui o socket que o windows atribuiu para a conexão
//O socket é utilizado para se comunicar tanto como cliente,como com o servidor.
void basic_connection::setSock(int32 sock)
{
    unique_lock<shared_mutex> lck(mtx_con);
    this->sock = sock;
}

int32 basic_connection::getSock() const
{
    shared_lock<shared_mutex> lck(mtx_con);
    return this->sock;
}

// Define o estado da conexão no nível do cliente (conectado ou desconectado)
void basic_connection::setConState(uint32 conState)
{
    unique_lock<shared_mutex> lck(mtx_con);
    this->conState = conState;
}

uint32 basic_connection::getConState() const
{
    shared_lock<shared_mutex> lck(mtx_con);
    return this->conState;
}

// Verifica se está conectado
bool basic_connection::is_connected() const
{
    shared_lock<shared_mutex> lck(mtx_con);
    return conState == con_connected;
}
