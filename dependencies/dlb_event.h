

/**
*Classe base para eventos. Também implementa uma fila para envio e recebimento de eventos.
*Escrito por Róger (rogerlasch@gmail.com) em Setembro de 2021.
*Este arquivo faz parte da dlb_utils, uma biblioteca de utilidades. Use por sua conta e risco!
*Atualizado em junho de 2022
**/


#ifndef DLB_EVENT_H
#define DLB_EVENT_H

namespace dlb
{


#define dlb_event_default 0

class dlb_event
{
public:
uint32 id;//Id do evento, se aplicável.
uint32 type;//Tipo do evento. Isto pode ser qualquer coisa que seu protocolo precise.
int64 timestamp;//A hora em millisegundos que o evento foi gerado.
std::string data;//Argumentos adicionais, se aplicável.
dlb_worker* worker;//O worker que capturou este evento...
dlb_event();
dlb_event(const dlb_event& dev);
dlb_event& operator=(const dlb_event& dev);
virtual ~dlb_event();
void reset();
};

//Os 2 métodos a seguir enfileiram um evento na fila.
void dlb_event_send(uint32 type, uint32 id, const std::string& data="");
void dlb_event_send(dlb_event* ev);
//Recupera um evento da fila...
bool dlb_event_get(dlb_event** ev);
uint32 dlb_event_count();
void dlb_event_cleanup();
}
#endif
