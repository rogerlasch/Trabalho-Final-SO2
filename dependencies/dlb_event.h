

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
uint32 id;
uint32 type;
int64 timestamp;
std::string data;
dlb_worker* worker;//O worker que capturou este evento...
dlb_event();
dlb_event(const dlb_event& dev);
dlb_event& operator=(const dlb_event& dev);
virtual ~dlb_event();
void reset();
};

void dlb_event_send(uint32 type, uint32 id, const std::string& data="");
void dlb_event_send(dlb_event* ev);
bool dlb_event_get(dlb_event** ev);
uint32 dlb_event_count();
void dlb_event_cleanup();
}
#endif
