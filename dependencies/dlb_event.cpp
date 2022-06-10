

#include<list>
#include<shared_mutex>
#include<mutex>
#include"ed3_lib.h"
#include"dlb_worker.h"
#include"dlb_event.h"


using namespace std;
namespace dlb
{
static list<dlb_event*> dlb_events;
static shared_mutex mtx_event;

dlb_event::dlb_event()
{
this->reset();
}

dlb_event::dlb_event(const dlb_event& dev)
{
*this=dev;
}

dlb_event& dlb_event::operator=(const dlb_event& dev)
{
this->id=dev.id;
this->type=dev.type;
this->timestamp=dev.timestamp;
return *this;
}

dlb_event::~dlb_event()
{
}

void dlb_event::reset()
{
this->id=0;
this->type=dlb_event_default;
this->timestamp=0;
this->worker=NULL;
}


//Functions

void dlb_event_send(uint32 type, uint32 id)
{
dlb_event* ev=new dlb_event();
ev->id=id;
ev->type=type;
dlb_event_send(ev);
}

void dlb_event_send(dlb_event* ev)
{
if(ev==NULL)
{
return;
}
unique_lock<shared_mutex> lck(mtx_event);
ev->timestamp=gettimestamp();
dlb_events.push_back(ev);
if(dlb_events.size()==1)
{
dlb_worker_set_can_state(dlb_worker_can_work);
}
}

bool dlb_event_get(dlb_event** ev)
{
if(ev==NULL)
{
return false;
}
unique_lock<shared_mutex> lck(mtx_event);
if(dlb_events.size()==0)
{
return false;
}
*ev=*dlb_events.begin();
dlb_events.erase(dlb_events.begin());
if(dlb_events.size()==0)
{
dlb_worker_set_can_state(dlb_worker_can_pause);
}
return true;
}

uint32 dlb_event_count()
{
shared_lock<shared_mutex> lck(mtx_event);
return dlb_events.size();
}

void dlb_event_cleanup()
{
unique_lock<shared_mutex> lck(mtx_event);
for(auto it=dlb_events.begin(); it!=dlb_events.end(); ++it)
{
delete (*it);
}
dlb_events.clear();
}

}
