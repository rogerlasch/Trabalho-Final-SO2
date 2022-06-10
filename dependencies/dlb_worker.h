
/**
*Classe worker, é similar a dlb_task,mas este trabalha especificamente em conjunto com dlb_event.
*Também tem funções para manipular um pool de workers.
*Eles são úteis para lidar com a fila de eventos da dlb. Você informa uma função para processar os eventos e quantos workers quer que cuidem da fila, e deixe o resto por conta deles.
*Escrito por Róger (rogerlasch@gmail.com) em setembro de 2021.
*Este arquivo faz parte da dlb_utils, uma biblioteca de utilidades. Use por sua conta e risco!
*Atualizado em março e junho de 2022
**/

#ifndef DLB_WORKER_H
#define DLB_WORKER_H

#include<future>
#include<functional>
#include"dlb_basic_flags.h"

namespace dlb
{

#ifndef DLB_EVENT_H
class dlb_event;
#endif
typedef std::function<void(dlb_event* ev)> dlb_event_callback;

enum dlb_worker_can_work_returns
{
dlb_worker_can_stop=0,//Interrompa o trabalho a sim que puder...
dlb_worker_can_checkup,//Acorde e verifique se não é você  que deve ser encerrado.
dlb_worker_can_pause,//Não à mais trabalho no momento, vá dormir!
dlb_worker_can_work//Acorde, tem trabalho à ser feito!
};

enum dlb_worker_flags
{
dlb_worker_paused=(1<<0),
dlb_worker_active=(1<<1),
//Não usar esta flag...
dlb_worker_stop_work=(1<<2)
};

enum dlb_worker_stop_result
{
dlb_worker_deferred=std::future_status::deferred,
dlb_worker_ready=std::future_status::ready,
dlb_worker_timeout=std::future_status::timeout,
dlb_worker_handle//Handle inválido
};

class dlb_worker_info
{
public:
uint64 events;//Número de eventos que processou...
int64 total_executiontime;//O tempo total em que de fato processou eventos.
int64 average_executiontime;//O tempo médio de execução de cada evento
dlb_worker_info();
dlb_worker_info(const dlb_worker_info& info);
dlb_worker_info& operator=(const dlb_worker_info& info);
void reset();
};

class dlb_worker : public dlb_basic_flags
{
private:
uint32 id;
std::string name;
dlb_worker_info winfo;
dlb_event_callback evcall;
std::future<void> handle;
mutable std::shared_mutex mtx;
public:
dlb_worker(uint32 id=0, uint32 flags=0, const std::string& name="", dlb_event_callback evcall=NULL);
dlb_worker(const dlb_worker& dw)=delete;
dlb_worker& operator=(const dlb_worker& dw)=delete;
virtual ~dlb_worker();
void setId(uint32 id);
uint32 getId()const;
void setName(const std::string& name);
std::string getName()const;
void setEvCall(dlb_event_callback evcall);
dlb_event_callback getEvCall()const;
bool getInfo(dlb_worker_info* info);
uint32 stop(uint32 timeout=0);
uint32 pause();
bool resume();
bool isPaused()const;
bool isStopped()const;
bool isActive()const;
private:
bool continueLoop()const;
void workerLoop();
};

void dlb_worker_set_can_state(uint32 wstate);
uint32 dlb_worker_get_can_state();
bool dlb_worker_is_awakening();
uint32 dlb_worker_create(const std::string& name, uint32 flags, dlb_event_callback evcall);
uint32 dlb_worker_create(uint32 n_workers, uint32 flags, dlb_event_callback evcall);
uint32 dlb_worker_create(uint32 n_workers, uint32 flags, dlb_event_callback evcall, std::vector<uint32>& ids);
bool dlb_worker_exists(uint32 id);
uint32 dlb_worker_count();
uint32 dlb_worker_stop_n(uint32 id);
uint32 dlb_worker_stop(uint32 n_workers);
uint32 dlb_worker_pause(uint32 id);
bool dlb_worker_resume(uint32 id);
bool dlb_worker_ispaused(uint32 id);
bool dlb_worker_isstopped(uint32 id);
uint32 dlb_worker_stop_all();
uint32 dlb_worker_stop_all(std::vector<dlb_worker_info>& winfo);
uint32 dlb_worker_pause_all();
uint32 dlb_worker_resume_all();
bool dlb_worker_getinfo(uint32 id, dlb_worker_info* winfo);

}
#endif
