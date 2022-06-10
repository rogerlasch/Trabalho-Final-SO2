

#include<unordered_map>
#include"../dependencies/so2_includes.h"
#include"basic_connection.h"

using namespace std;
using namespace dlb;

basic_connection::basic_connection()
{
sock=0;
conState=0;
cmd_line="";
input_buffer.clear();
output_buffer.clear();
}

basic_connection::~basic_connection()
{
}

void basic_connection::print(const string& str)
{
//Pega direito de escrita j� que algo vai ser modificado...
unique_lock<shared_mutex> lck(this->mtx_output);
output_buffer.push_back(str);
}

string basic_connection::get_line_to_send()
{
unique_lock<shared_mutex> lck(this->mtx_output);
string str="";
if(output_buffer.size()>0)
{
str=output_buffer[0];
output_buffer.erase(output_buffer.begin());
}
return str;
}

void basic_connection::append_string_input(const string& str)
{
unique_lock<shared_mutex> lck(this->mtx_input);
for(uint32 i=0; i<str.size(); i++)
{
if((str[i]=='\0')||(str[i]=='\r')||(str[i]=='\n'))
{
if(cmd_line.size()>0)
{
input_buffer.push_back(cmd_line);
cmd_line.resize(0);
}
}
else
{
cmd_line+=str[i];
}
}
}

void basic_connection::process_input()
{
unique_lock<shared_mutex> lck(this->mtx_input);
if(input_buffer.size()>0)
{
while(input_buffer.size()>0)
{
//Despacha um evento para os workers darem um jeito...
dlb_event_send(event_receive, getSock(), input_buffer[0]);
input_buffer.erase(input_buffer.begin());
}
}
}

void basic_connection::setSock(int32 sock)
{
unique_lock<shared_mutex> lck(mtx_con);
this->sock=sock;
}

int32 basic_connection::getSock()const
{
shared_lock<shared_mutex> lck(mtx_con);
return this->sock;
}

void basic_connection::setConState(uint32 conState)
{
unique_lock<shared_mutex> lck(mtx_con);
this->conState=conState;
}

uint32 basic_connection::getConState()const
{
shared_lock<shared_mutex> lck(mtx_con);
return this->conState;
}

bool basic_connection::is_connected()const
{
shared_lock<shared_mutex> lck(mtx_con);
return conState==con_connected;
}
