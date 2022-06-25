


#ifndef TABLE_H
#define TABLE_H

enum GAME_STATE
{
g_starting=0,
g_playing,
g_finished
};

enum TURN_DIR
{
turn_left=0,
turn_right
};

class Table
{
private:
uint32 id;
std::atomic<uint32> current_turn;
std::atomic<uint32> gstate;
std::atomic<uint32> pindex;
std::atomic<uint32> turn_dir;
std::vector<shared_player> players;
Deck deck;
Deck discard;
Deck acumulator;
shared_player current_player;
shared_card current_card;
public:
Table();
Table(const Table& t)=delete;
Table& operator=(const Table& t);
~Table();
void setId(uint32 id);
uint32 getId()const;
shared_player find_player(uint32 sock);
shared_player find_player(const std::string& name);
void add_player(const shared_player& c);
uint32 get_gstate()const;
void generate_cards();
bool start_game();
void process_command(shared_player& ch, const std::string& cmdline);
private:
void swap_turn();
shared_player next_player(bool jump_one=false);
};
typedef std::shared_ptr<Table> shared_table;
typedef std::vector<shared_table> table_list;

uint32 table_generate_id();
#endif

#ifndef TABLE_IMPLEMENTATION
#define TABLE_IMPLEMENTATION

Table::Table()
{
this->setId(table_generate_id());
current_turn.store(0);
gstate.store(g_starting);
pindex.store(0);
turn_dir.store(turn_right);
players.clear();
deck.clear();
discard.clear();
acumulator.clear();
current_player=NULL;
current_card=NULL;
}

Table::~Table()
{
}

void Table::setId(uint32 id)
{
this->id=id;
}

uint32 Table::getId()const
{
return this->id;
}

shared_player Table::find_player(uint32 sock)
{
if((sock==0)||(players.size()==0))
{
return shared_player();
}
for(auto it=players.begin(); it!=players.end(); ++it)
{
if((*it)->getSock()==sock)
{
return *it;
}
}
return shared_player();
}

shared_player Table::find_player(const std::string& name)
{
if((name.size()==0)||(players.size()==0))
{
return shared_player();
}
for(auto it=players.begin(); it!=players.end(); ++it)
{
if((*it)->getName()==name)
{
return *it;
}
}
return shared_player();
}

void Table::add_player(const shared_player& c)
{
players.push_back(c);
}

uint32 Table::get_gstate()const
{
return gstate.load();
}

void Table::generate_cards()
{
}

bool Table::start_game()
{
return false;
}

void Table::process_command(shared_player& ch, const std::string& cmdline)
{
}

void Table::swap_turn()
{
}

shared_player Table::next_player(bool jump_one)
{
return shared_player();
}

//Funções...
uint32 table_generate_id()
{
static uint32 x=0;
x++;
return x;
}
#endif
