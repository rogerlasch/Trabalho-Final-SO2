


#include<condition_variable>
#include<unordered_map>
#include"ed3_lib.h"
#include"dlb_worker.h"
#include"dlb_event.h"

using namespace std;
namespace dlb
{
static uint32 dlb_worker_state=0;
static unordered_map<uint32, dlb_worker*> dlb_workers;
static shared_mutex mtx_workers;
static shared_mutex mtx_wstate;
static condition_variable cv_workers;

dlb_worker_info::dlb_worker_info()
{
this->reset();
}

dlb_worker_info::dlb_worker_info(const dlb_worker_info& info)
{
*this=info;
}

dlb_worker_info& dlb_worker_info::operator=(const dlb_worker_info& info)
{
this->events=info.events;
this->total_executiontime=info.total_executiontime;
this->average_executiontime=info.average_executiontime;
return *this;
}

void dlb_worker_info::reset()
{
this->events=0;
this->total_executiontime=0;
this->average_executiontime=0;
}

//The dlb_worker class

dlb_worker::dlb_worker(uint32 id, uint32 flags, const string& name, dlb_event_callback evcall)
:dlb_basic_flags(0)
{
setName(name);
setId(id);
setEvCall(evcall);
this->replace_flags(flags);
if(!flag_contains(dlb_worker_paused))
{
this->setflag(dlb_worker_paused);
this->resume();
}
}

dlb_worker::~dlb_worker()
{
if(this->isActive())
{
this->stop(100);
}
}

void dlb_worker::setId(uint32 id)
{
unique_lock<shared_mutex> lck(mtx);
this->id=id;
}

uint32 dlb_worker::getId()const
{
shared_lock<shared_mutex> lck(this->mtx);
return this->id;
}

void dlb_worker::setName(const string& name)
{
unique_lock<shared_mutex> lck(mtx);
this->name=name;
}

string dlb_worker::getName()const
{
shared_lock<shared_mutex> lck(this->mtx);
return this->name;
}

void dlb_worker::setEvCall(dlb_event_callback evcall)
{
unique_lock<shared_mutex> lck(mtx);
this->evcall=evcall;
}

dlb_event_callback dlb_worker::getEvCall()const
{
shared_lock<shared_mutex> lck(this->mtx);
return this->evcall;
}

bool dlb_worker::getInfo(dlb_worker_info* info)
{
if(info==NULL)
{
return false;
}
*info=winfo;
return true;
}

uint32 dlb_worker::stop(uint32 timeout)
{
if(this->isStopped())
{
return dlb_worker_ready;
}
this->setflag(dlb_worker_stop_work);
cv_workers.notify_all();
return static_cast<uint32>(this->handle.wait_for(chrono::milliseconds(timeout)));
}

uint32 dlb_worker::pause()
{
if(this->isPaused())
{
return dlb_worker_ready;
}
this->setflag(dlb_worker_paused);
cv_workers.notify_all();
uint32 result=static_cast<uint32>(this->handle.wait_for(chrono::milliseconds(20)));
if(result!=dlb_worker_ready)
{
this->removeflag(dlb_worker_paused);
}
return result;
}

bool dlb_worker::resume()
{
if((this->flag_contains(dlb_worker_stop_work))||(!this->isPaused()))
{
return false;
}
unique_lock<shared_mutex> lck(this->mtx);
this->removeflag(dlb_worker_paused);
this->removeflag(dlb_worker_stop_work);
this->handle=async(&dlb_worker::workerLoop, this);
return true;
}

bool dlb_worker::isPaused()const
{
return flag_contains(dlb_worker_paused);
}

bool dlb_worker::isStopped()const
{
return !flag_contains(dlb_worker_active);
}

bool dlb_worker::isActive()const
{
return flag_contains(dlb_worker_active);
}

bool dlb_worker::continueLoop()const
{
return ((this->flag_contains(dlb_worker_paused)||this->flag_contains(dlb_worker_stop_work)||!this->flag_contains(dlb_worker_active)) ? false : true);
}

void dlb_worker::workerLoop()
{
this->setflag(dlb_worker_active);
mutex mtx_work;
unique_lock<mutex> lck(mtx_work);
while(this->continueLoop())
{
switch(dlb_worker_get_can_state())
{
case dlb_worker_can_stop:
case dlb_worker_can_pause:
case dlb_worker_can_checkup:
{
if(this->continueLoop())
{
cv_workers.wait(lck);
}
break;
}
case dlb_worker_can_work:
{
dlb_event* ev=NULL;
if(dlb_event_get(&ev))
{
unique_lock<shared_mutex> lck(this->mtx);
winfo.events++;
if(evcall)
{
ev->worker=this;
int64 start=gettimestamp();
evcall(ev);
int64 end=gettimestamp();
winfo.total_executiontime+=(end-start);
winfo.average_executiontime=(winfo.total_executiontime/winfo.events);
}
lck.unlock();
delete ev;
ev=NULL;
}
break;
}
}
}
removeflag(dlb_worker_active);
}

//Functions...

void dlb_worker_set_can_state(uint32 wstate)
{
unique_lock<shared_mutex> lck(mtx_wstate);
dlb_worker_state=wstate;
cv_workers.notify_all();
}

uint32 dlb_worker_get_can_state()
{
shared_lock<shared_mutex> lck(mtx_wstate);
return dlb_worker_state;
}

bool dlb_worker_is_awakening()
{
uint32 x=dlb_worker_get_can_state();
return x==dlb_worker_can_checkup||dlb_worker_can_work;
}

static uint32 dlb_worker_generate_id()
{
static int x=0;
x++;
return x;
}

uint32 dlb_worker_create(const std::string& name, uint32 flags, dlb_event_callback evcall)
{
uint32 id=dlb_worker_generate_id();
dlb_worker* wk=new dlb_worker(id, flags, name, evcall);
unique_lock<shared_mutex> lck(mtx_workers);
dlb_workers.insert(make_pair(id, wk));
return id;
}

uint32 dlb_worker_create(uint32 n_workers, uint32 flags, dlb_event_callback evcall)
{
for(uint32 i=0; i<n_workers; i++)
{
dlb_worker_create("", flags, evcall);
}
return n_workers;
}

uint32 dlb_worker_create(uint32 n_workers, uint32 flags, dlb_event_callback evcall, std::vector<uint32>& ids)
{
ids.clear();
ids.reserve(n_workers);
for(uint32 i=0; i<n_workers; i++)
{
ids.push_back(dlb_worker_create("", flags, evcall));
}
return n_workers;
}

bool dlb_worker_exists(uint32 id)
{
shared_lock<shared_mutex> lck(mtx_workers);
return dlb_workers.find(id)!=dlb_workers.end();
}

uint32 dlb_worker_count()
{
shared_lock<shared_mutex> lck(mtx_workers);
return dlb_workers.size();
}

uint32 dlb_worker_stop(uint32 id)
{
if(!dlb_worker_exists(id))
{
return dlb_worker_handle;
}
unique_lock<shared_mutex> lck(mtx_workers);
dlb_worker* wk=dlb_workers.at(id);
uint32 res=wk->stop(20);
if(res==dlb_worker_ready)
{
dlb_workers.erase(id);
delete wk;
}
return res;
}

uint32 dlb_worker_stop_n(uint32 n_workers)
{
if((n_workers==0)||(dlb_worker_count()==0))
{
return 0;
}
uint32 x=0;
if(dlb_worker_count()<n_workers)
{
n_workers=dlb_worker_count();
}
for(uint32 i=0; i<n_workers; i++)
{
if(dlb_worker_count()>0)
{
if(dlb_worker_isstopped(dlb_workers.begin()->first))
{
dlb_worker_stop(dlb_workers.begin()->first);
if(i>0)
{
i--;
}
continue;
}
if(dlb_worker_stop(dlb_workers.begin()->first)==dlb_worker_ready)
{
x++;
}
}
}
return x;
}

uint32 dlb_worker_pause(uint32 id)
{
if(!dlb_worker_exists(id))
{
return dlb_worker_handle;
}
shared_lock<shared_mutex> lck(mtx_workers);
dlb_worker* wk=dlb_workers.at(id);
return wk->pause();
}

bool dlb_worker_resume(uint32 id)
{
if(!dlb_worker_exists(id))
{
return false;
}
shared_lock<shared_mutex> lck(mtx_workers);
dlb_worker* wk=dlb_workers.at(id);
return wk->resume();
}

bool dlb_worker_ispaused(uint32 id)
{
if(!dlb_worker_exists(id))
{
return false;
}
shared_lock<shared_mutex> lck(mtx_workers);
return dlb_workers.at(id)->isPaused();
}

bool dlb_worker_isstopped(uint32 id)
{
if(!dlb_worker_exists(id))
{
return false;
}
shared_lock<shared_mutex> lck(mtx_workers);
return dlb_workers.at(id)->isStopped();
}

uint32 dlb_worker_stop_all()
{
uint32 x=0;
while(dlb_worker_count()>0)
{
if(dlb_worker_stop(dlb_workers.begin()->first)==dlb_worker_ready)
{
x++;
}
}
return x;
}

uint32 dlb_worker_stop_all(vector<dlb_worker_info>& winfo)
{
uint32 x=0;
while(dlb_worker_count()>0)
{
dlb_worker_info wf;
dlb_workers.begin()->second->getInfo(&wf);
if(dlb_worker_stop(dlb_workers.begin()->first)==dlb_worker_ready)
{
x++;
winfo.push_back(wf);
}
}
return x;
}

uint32 dlb_worker_pause_all()
{
uint32 x=0;
for(auto it=dlb_workers.begin(); it!=dlb_workers.end(); ++it)
{
if(dlb_worker_pause(it->first)==dlb_worker_ready)
{
x++;
}
}
return x;
}

uint32 dlb_worker_resume_all()
{
uint32 x=0;
for(auto it=dlb_workers.begin(); it!=dlb_workers.end(); ++it)
{
if(dlb_worker_resume(it->first))
{
x++;
}
}
return x;
}

bool dlb_worker_getinfo(uint32 id, dlb_worker_info*winfo)
{
if(!dlb_worker_exists(id))
{
return false;
}
shared_lock<shared_mutex> lck(mtx_workers);
return dlb_workers.at(id)->getInfo(winfo);
}
}
