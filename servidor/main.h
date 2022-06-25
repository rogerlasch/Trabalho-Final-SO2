



#ifndef MAIN_H
#define MAIN_H

enum SERVER_STATES
{
server_starting=0,
server_running,
server_shuting_down,
server_waiting_exit
};

struct StringUtils
{
void parse(const std::string& str, std::string& cmd, std::string& args)
{
std::string s=this->trim(str);
int32 x=s.find(' ', 0);
if(x==std::string::npos)
{
cmd=to_lower_case(s);
return;
}
cmd=to_lower_case(s.substr(0, x));
args=trim(s.substr(x, s.size()));
}
std::string smash(const std::string& str, const std::string& char_set)
{
std::string final="";
for(uint32 i=0; i<str.size(); i++)
{
if(char_set.find(str[i], 0)!=std::string::npos)
{
continue;
}
final+=str[i];
}
return final;
}
std::string to_lower_case(const std::string& str)
{
   std::string final="";
for(uint32 i=0; i<str.size(); i++)
{
final+=std::tolower(str[i]);
}
return final;
}
std::string trim(const std::string& s)
{
std::string t=" \t\r\n";
std::string d=s;
std::string::size_type i=d.find_last_not_of (t);
if(i==std::string::npos)
return "";
else
   return d.erase (i + 1).erase (0, s.find_first_not_of (t)) ;
}
};

void s_setState(uint32 st);
uint32 s_getState();
//As próximas funções são comandos. Não devem ser invocadas diretamente.
void do_help(shared_player& ch, const std::string& args);
void do_commands(shared_player& ch, const std::string& args);
void do_create_party(shared_player& ch, const std::string& args);
void do_chat(shared_player& ch, const std::string& args);
void do_quit(shared_player& ch, const std::string& args);
void do_who(shared_player& ch, const std::string& args);
#endif
