


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

class Table : public std::enable_shared_from_this<Table>
{
private:
typedef std::function<void(shared_player&, const std::string&)> table_command;
std::map<std::string, table_command> cmdtable;
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
uint32 playerCount()const;
std::string toString();
shared_player find_player(uint32 sock);
shared_player find_player(const std::string& name);
void add_player(const shared_player& c);
void remove_player(const std::string& name);
uint32 get_gstate()const;
void generate_cards();
void process_command(shared_player& ch, const std::string& cmdline);
private:
void swap_turn();
shared_player next_player(bool jump_one=false);
//NÃO INVOCAR OS PRÓXIMOS MÉTODOS DIRETAMENTE!!!!
void commands(shared_player& ch, const std::string& args);
void goBack(shared_player& ch, const std::string& args);
void startGame(shared_player& ch, const std::string& args);
void playCard(shared_player& ch, const std::string& args);
void toFish(shared_player& ch, const std::string& args);
};
typedef std::shared_ptr<Table> shared_table;
typedef std::vector<shared_table> table_list;

uint32 table_generate_id();
void t_send_to_list(std::vector<shared_player>& players, const std::string& msg);
#define _echo(cons, str, ...) t_send_to_list(cons, fmt::format(str, __VA_ARGS__))
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
cmdtable={
{"comandos", std::bind(&Table::commands, this, std::placeholders::_1, std::placeholders::_2)},
{"voltar", std::bind(&Table::goBack, this, std::placeholders::_1, std::placeholders::_2)},
{"startgame", std::bind(&Table::startGame, this, std::placeholders::_1, std::placeholders::_2)},
{"jogar", std::bind(&Table::playCard, this, std::placeholders::_1, std::placeholders::_2)},
{"pescar", std::bind(&Table::toFish, this, std::placeholders::_1, std::placeholders::_2)}
};
}

Table::~Table()
{
cmdtable.clear();
}

void Table::setId(uint32 id)
{
this->id=id;
}

uint32 Table::getId()const
{
return this->id;
}

uint32 Table::playerCount()const
{
return players.size();
}

std::string Table::toString()
{
std::stringstream ss;
ss<<"Id: "<<this->getId()<<", ";
ss<<"Players: "<<players.size()<<", ";
ss<<"Status: ";
switch(this->get_gstate())
{
case g_starting:
{
ss<<"Aguardando jogadores";
break;
}
case g_playing:
{
ss<<"Jogando";
break;
}
case g_finished:
{
ss<<"Jogo finalizado";
break;
}
}
return ss.str();
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
c->setPState(player_expectator);
c->setTable(this->shared_from_this());
c->print(fmt::format("Bem-vindo a mesa {} {}.", this->getId(), c->getName()));
if(players.size()==1)
{
c->print("Digite o comando \"startgame\" para iniciar a partida quando tiver pelo menos mais um jogador.");
}
_echo(players, "{} se juntou a mesa.", c->getName());
}

void Table::remove_player(const std::string& name)
{
for(uint32 i=0; i<players.size(); i++)
{
if(players[i]->getName()==name)
{
auto ch=players[i];
ch->print("Até logo...");
ch->setTable(shared_table());
players.erase(players.begin()+i);
ch->print("Digite comandos para ver uma lista de comandos disponíveis.");
}
}
}

uint32 Table::get_gstate()const
{
return gstate.load();
}

void Table::generate_cards()
{
deck.clear();
struct CardCreator
{
void operator()(Deck& d, uint32 type, uint32 color, uint32 number, uint32 qtd)
{
for(uint32 i=0; i<qtd; i++)
{
shared_card c=std::make_shared<Card>();
c->setType(type);
c->setColor(color);
c->setNumber(number);
d.push_back(c);
}
}
};
CardCreator ct;
for(uint32 number=0; number<=9; number++)
{
for(uint32 color=red; color<=blue; color++)
{
ct(deck, normal, color, number, ((number==0) ? 1 : 2));
}
}
for(uint32 ctype=block; ctype<=plus_two; ctype++)
{
for(uint32 color=red; color<=blue; color++)
{
ct(deck, ctype, color, 0, 2);
}
}
ct(deck, plus_four, uncolor, 0, 4);
ct(deck, joker, uncolor, 0, 4);
_echo(players, "Cartas geradas: {}", deck.size());
for(auto& c: deck)
{
_log("{}", c->toString());
}
}

void Table::process_command(shared_player& ch, const std::string& cmdline)
{
assert(ch!=NULL);
assert(ch->getTable()!=NULL);
if(this->getId()!=ch->getTable()->getId())
{
ch->print("Você está na mesa errada, man!");
return;
}
StringUtils cs;
std::string cmd="", args="";
cs.parse(cmdline, cmd, args);
//Pesquisa na tabela de comandos para determinar se ele digitou algum comando válido.
auto it=cmdtable.find(cmd);
//Caso nenhum comando tenha sido encontrado...
if(it==cmdtable.end())
{
ch->print("O que?");
return;
}
//Chame o comando que o jogador digitou passando os argumentos...
it->second(ch, args);
}

void Table::swap_turn()
{
}

shared_player Table::next_player(bool jump_one)
{
FuncTimer ts(__FUNCTION__);
bool achou=false;
uint32 x=pindex.load()+1;
shared_player ch;
while(x!=pindex)
{
if(x>players.size()-1)
{
x=0;
continue;
}
if(players[x]->getPState()==player_expectator)
{
x++;
continue;
}
ch=players[x];
break;
}
current_player=ch;
pindex.store(x);
return ch;
}

void Table::commands(shared_player& ch, const std::string& args)
{
std::stringstream ss;
ss<<"Comandos disponíveis:"<<std::endl;
for(auto it=cmdtable.begin(); it!=cmdtable.end(); ++it)
{
ss<<it->first<<std::endl;
}
ss<<cmdtable.size()<<" comandos encontrados."<<std::endl;
ch->print(ss.str());
}

void Table::goBack(shared_player& ch, const std::string& args)
{
for(uint32 i=0; i<players.size(); i++)
{
if(players[i]->getName()==ch->getName())
{
ch->print("Até logo...");
ch->setTable(shared_table());
players.erase(players.begin()+i);
ch->print("Digite comandos para ver uma lista de comandos disponíveis.");
_echo(players, "{} deixou a mesa.", ch->getName());
}
}
}

void Table::startGame(shared_player& ch, const std::string& args)
{
if(this->get_gstate()!=g_starting)
{
return;
}
if(players.size()<2)
{
ch->print("Ainda não existem jogadores o suficiente para dar início a partida.");
return;
}
this->gstate.store(g_playing);
for(auto& it : players)
{
it->setPState(player_playing);
it->dropCards();
}
_echo(players, "{} iniciou o jogo.\nIniciando embaralhamento e distribuição das cartas...", ch->getName());
this->generate_cards();
//Embaralhar as cartas...
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(deck.begin(), deck.end(), g);
for(auto& ch : players)
{
//Expectadores não recebem cartas!
if(ch->getPState()==player_expectator)
{
continue;
}
for(uint32 i=0; i<7; i++)
{
int32 x=random_int32(0, deck.size()-1);
ch->add_card(deck[x]);
deck.erase(deck.begin()+x);
}
ch->showCards();
}
turn_dir.store(turn_right);
current_card=deck[0];
deck.erase(deck.begin());
current_player=players[0];
pindex.store(0);
_echo(players, "A carta virada foi: {}", current_card->toString());
_echo(players, "{} começa o jogo.", current_player->getName());
}

void Table::playCard(shared_player& ch, const std::string& args)
{
if(ch->getSock()==current_player->getSock())
{
ch->print("Aguarde sua vez de jogar!");
return;
}
uint32 index=std::atoi(args.c_str());
if(index==0)
{
ch->print("Opção inválida!");
return;
}
shared_card c=ch->remove_card(index-1);
_echo(players, "{} jogou {}", ch->getName(), c->toString());
discard.push_back(c);
ch->showCards();
this->next_player();
_echo(players, "É a vez de {} jogar.", current_player->getName());
current_player->showCards();
}

void Table::toFish(shared_player& ch, const std::string& args)
{
}

//Funções...
uint32 table_generate_id()
{
static uint32 x=0;
x++;
return x;
}

void t_send_to_list(std::vector<shared_player>& cons, const std::string& msg)
{
for(auto& it: cons)
{
it->print(msg);
}
}


#endif
