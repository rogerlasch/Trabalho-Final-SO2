


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
void interact_bot();
uint32 calculate_plus_cards()const;
private:
shared_player next_player();
void internal_PlayCard(shared_player& ch, shared_card& c, uint32 index);
//N�O INVOCAR OS PR�XIMOS M�TODOS DIRETAMENTE!!!!
void commands(shared_player& ch, const std::string& args);
void goBack(shared_player& ch, const std::string& args);
void startGame(shared_player& ch, const std::string& args);
void playCard(shared_player& ch, const std::string& args);
void toFish(shared_player& ch, const std::string& args);
void jumpChange(shared_player& ch, const std::string& args);
void cmd_bot(shared_player& ch, const std::string& args);
};
typedef std::shared_ptr<Table> shared_table;
typedef std::vector<shared_table> table_list;

uint32 table_generate_id();
void t_send_to_list(std::vector<shared_player>& players, uint32 ignore_sock, const std::string& msg);
#define _echo(cons, ignore_sock, str, ...) t_send_to_list(cons, ignore_sock, fmt::format(str, __VA_ARGS__))
#endif

#ifndef TABLE_IMPLEMENTATION
#define TABLE_IMPLEMENTATION
#include"Bot.h"

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
{"pescar", std::bind(&Table::toFish, this, std::placeholders::_1, std::placeholders::_2)},
{"pular_vez", std::bind(&Table::jumpChange, this, std::placeholders::_1, std::placeholders::_2)},
{"bot", std::bind(&Table::cmd_bot, this, std::placeholders::_1, std::placeholders::_2)},
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
_echo(players, 0, "{} se juntou a mesa.", c->getName());
}

void Table::remove_player(const std::string& name)
{
for(uint32 i=0; i<players.size(); i++)
{
if(players[i]->getName()==name)
{
auto ch=players[i];
if(ch->getPState()==player_playing)
{
Deck d=ch->getDeck();
for(auto& it : d)
{
deck.insert(deck.begin()+random_int32(0, deck.size()-1), it);
}
if(pindex.load()==i)
{
next_player();
_echo(players, 0, "� a vez de {} jogar.", current_player->getName());
current_player->showCards();
if(pindex.load()>i)
{
pindex.fetch_sub(1);
}
}
else if(pindex.load()>i)
{
pindex.fetch_sub(1);
}
}
ch->print("At� logo...");
ch->setTable(shared_table());
players.erase(players.begin()+i);
ch->print("Digite comandos para ver uma lista de comandos dispon�veis.");
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
_echo(players, 0, "Cartas geradas: {}", deck.size());
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
ch->print("Voc� est� na mesa errada, man!");
return;
}
StringUtils cs;
std::string cmd="", args="";
cs.parse(cmdline, cmd, args);
//Pesquisa na tabela de comandos para determinar se ele digitou algum comando v�lido.
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

uint32 Table::calculate_plus_cards()const
{
if(acumulator.size()==0)
{
return 0;
}
uint32 x=0;
for(auto& it : acumulator)
{
switch(it->getType())
{
case plus_two:
{
x+=2;
break;
}
case plus_four:
{
x+=4;
break;
}
}
}
return x;
}

shared_player Table::next_player()
{
shared_player ch;
uint32 x=0;
switch(turn_dir.load())
{
case turn_right:
{
x=pindex.load()+1;
bool jumped=false;
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
if((current_card->getType()==block)&&(jumped==false))
{
jumped=true;
_echo(players, 0, "A vez de {} foi pulada", players[x]->getName());
x++;
continue;
}
ch=players[x];
break;
}
break;
}
case turn_left:
{
x=pindex.load();
bool jumped=false;
while((x-1)!=pindex)
{
if(x==0)
{
x=players.size();
continue;
}
if(players[x-1]->getPState()==player_expectator)
{
x--;
continue;
}
if((current_card->getType()==block)&&(jumped==false))
{
jumped=true;
_echo(players, 0, "A vez de {} foi pulada", players[x-1]->getName());
x--;
continue;
}
ch=players[x-1];
x--;
break;
}
break;
}
}
if(ch!=NULL)
{
current_player=ch;
pindex.store(x);
}
return ch;
}

void Table::internal_PlayCard(shared_player& ch, shared_card& c, uint32 index)
{
_echo(players, 0, "{} jogou {}", ch->getName(), c->toString());
discard.push_back(c);
current_card=discard[discard.size()-1];
ch->remove_card(index);
switch(c->getType())
{
case plus_two:
case plus_four:
{
acumulator.push_back(c);
_echo(players, 0, "Acumulando {}. Total de cartas acumuladas: {}", c->toString(), calculate_plus_cards());
break;
}
case reverse_turn:
{
if(turn_dir.load()==turn_right)
{
turn_dir.store(turn_left);
}
else
{
turn_dir.store(turn_right);
}
_echo(players, 0, "O turno foi invertido para o lado oposto.");
break;
}
}
//ch->showCards();
this->next_player();
_echo(players, 0, "� a vez de {} jogar.", current_player->getName());
current_player->showCards();
if(acumulator.size()>0)
{
CardFinder sf;
int32 x=calculate_plus_cards();
if(sf.find_card_type(current_player->getDeck(), {plus_two, plus_four})==-1)
{
_echo(players, 0, "{} n�o tem nenhum +2 ou +4, por tanto, ter� que comprar {:+} cartas!", current_player->getName(), x);
acumulator.clear();
for(uint32 i=0; i<x; i++)
{
process_command(current_player, "pescar -s");
}
}
}
if(current_player->isBot())
{
dlb::dlb_event_send(150, this->getId(), "");
}
}

void Table::interact_bot()
{
FuncTimer sh(__FUNCTION__);
if(current_player->isPlayer())
{
return;
}
bool done=false;
shared_bot ch=dynamic_pointer_cast<Bot>(current_player);
uint32 fished=0;
while(done==false)
{
std::this_thread::sleep_for(std::chrono::milliseconds(1500));
uint32 result=0;
bot_args args;
result=ch->decision(current_card, &args);
switch(result)
{
case b_play:
case b_jump:
{
this->process_command(current_player, args.toString());
done=true;
break;
}
case b_buy:
{
if(fished>3)
{
this->process_command(current_player, "pular_vez");
done=true;
break;
}
this->process_command(current_player, "pescar -s");
fished++;
break;
}
}
}
}

//Comandos... N�o invocar diretamente.
void Table::commands(shared_player& ch, const std::string& args)
{
std::stringstream ss;
ss<<"Comandos dispon�veis:"<<std::endl;
for(auto it=cmdtable.begin(); it!=cmdtable.end(); ++it)
{
ss<<it->first<<std::endl;
}
ss<<cmdtable.size()<<" comandos encontrados."<<std::endl;
ch->print(ss.str());
}

void Table::goBack(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Utilize o comando voltar para sair da mesa e retornar para a sala principal.");
return;
}
for(uint32 i=0; i<players.size(); i++)
{
if(players[i]->getName()==ch->getName())
{
ch->print("At� logo...");
ch->setTable(shared_table());
players.erase(players.begin()+i);
ch->print("Digite comandos para ver uma lista de comandos dispon�veis.");
_echo(players, 0, "{} deixou a mesa.", ch->getName());
}
}
}

void Table::startGame(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Digite startgame para iniciar o jogo. Apenas o criador da sala pode iniciar o jogo. Caso o criador saia, o controle passar� para o pr�ximo da lista, se poss�vel.");
return;
}
if(this->get_gstate()!=g_starting)
{
return;
}
if(players.size()<2)
{
ch->print("Ainda n�o existem jogadores o suficiente para dar in�cio a partida.");
return;
}
if(ch->getSock()!=players[0]->getSock())
{
ch->print("Voc� n�o pode iniciar a partida.");
return;
}
this->gstate.store(g_playing);
for(auto& it : players)
{
it->setPState(player_playing);
it->dropCards();
}
_echo(players, 0, "{} iniciou o jogo.\nIniciando embaralhamento e distribui��o das cartas...", ch->getName());
this->generate_cards();
//Embaralhar as cartas...
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(deck.begin(), deck.end(), g);
for(auto& ch : players)
{
//Expectadores n�o recebem cartas!
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
for(uint32 i=0; i<deck.size(); i++)
{
if(deck[i]->getType()==normal)
{
current_card=deck[i];
deck.erase(deck.begin()+i);
break;
}
}
turn_dir.store(turn_right);
current_player=players[0];
pindex.store(0);
_echo(players, 0, "A carta virada foi: {}", current_card->toString());
_echo(players, 0, "{} come�a o jogo.", current_player->getName());
}

void Table::playCard(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use o comando jogar para jogar uma carta se poss�vel.");
ch->print("Uso: jogar >�ndice> <cor>");
ch->print("Onde <�ndice> � o �ndice da carta em sua m�o. Para saber o �ndice da carta, verifique o n�mero que aparece entre () antes do nome.");
ch->print("<Cor> � utilizado apenas com cartas especiais como +4 e coringa. As cores aceitas s�o Verde, Vermelho, Amarelo e Azul.");
ch->print("Exemplos:\n\"jogar 1\" Ir� jogar a primeira carta.");
ch->print("\"jogar 2 azul\" Ir� jogar a segunda carta, e selecionar� a cor azul caso ela seja um +4 ou coringa.");
return;
}
if(ch->getSock()!=current_player->getSock())
{
ch->print("Aguarde sua vez de jogar!");
return;
}
std::string  index_str="", color_str="";
StringUtils cs;
cs.parse(args, index_str, color_str);
uint32 index=std::atoi(index_str.c_str());
uint32 color_id=color_to_int(color_str);
if(index==0)
{
ch->print("Op��o inv�lida!");
return;
}
shared_card c=ch->get_card(index-1);
if(c==NULL)
{
ch->print("Esta carta n�o existe em seu baralho.");
return;
}
if(ch->isBot())
{
_log("Jogada de {}, args: {}", ch->getName(), color_str);
_log("�ltima carta: {} Carta tentada: {}", current_card->toString(), c->toString());
}
if(acumulator.size()>0)
{
CardFinder sf;
if((c->getType()!=plus_two)&&(c->getType()!=plus_four)&&(sf.find_card_type(ch->getDeck(), {plus_two, plus_four})>-1))
{
ch->print(fmt::format("Voc� n�o pode jogar esta carta no momento porque um {} est� ativo.", current_card->toString()));
return;
}
}
if((c->getType()==normal)&&(current_card->getType()==normal))
{
if((c->getColor()==current_card->getColor())||(c->getNumber()==current_card->getNumber()))
{
internal_PlayCard(ch, c, (index-1));
}
else
{
ch->print("Esta jogada � inv�lida!");
}
}
else if((c->getType()==plus_four)||(c->getType()==joker))
{
if(color_str.size()==0)
{
ch->print("A cor n�o foi informada. Repita o comando incluindo uma cor v�lida como terceiro par�metro.\nExemplo: \"jogar 1 verde\"");
return;
}
if((color_id<red)||(color_id>blue))
{
ch->print("Esta cor n�o � v�lida.");
return;
}
if((current_card->getType()==plus_two)||(current_card->getType()==plus_four))
{
if(c->getType()==joker)
{
ch->print("Esta jogada n�o � v�lida!");
return;
}
}
c->setColor(color_id);
this->internal_PlayCard(ch, c, index-1);
}
else if(c->getType()==plus_two)
{
if((c->getColor()==current_card->getColor())||(c->getType()==current_card->getType())||(current_card->getType()==plus_four))
{
internal_PlayCard(ch, c, index-1);
}
else
{
ch->print("Jogada inv�lida!");
}
}
else if((c->getType()==current_card->getType())||(c->getColor()==current_card->getColor()))
{
this->internal_PlayCard(ch, c, index-1);
}
else
{
ch->print("Carta inv�lida!");
}
}

void Table::toFish(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando pescar � utilizado para comprar cartas quando for sua vez de jogar.");
ch->print("Use \"jogar -s\" se n�o quiser que toda sua m�o seja mostrada novamente ao pescar a carta.");
return;
}
if((ch->isPlayer())&&(ch->getSock()!=current_player->getSock()))
{
ch->print("Espere sua vez!");
return;
}
if(deck.size()==0)
{
uint32 x=discard.size()-1;
for(uint32 i=0; i<discard.size(); i++)
{
switch(discard[i]->getType())
{
case plus_four:
case joker:
{
discard[i]->setColor(uncolor);
break;
}
}
deck.push_back(discard[i]);
}
deck.erase(discard.begin(), discard.begin()+x);
//Embaralhar as cartas...
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(deck.begin(), deck.end(), g);
}
ch->add_card(deck[0]);
if(ch->isPlayer())
{
ch->print(fmt::format("Voc� comprou \"{}\"", deck[0]->toString()));
}
deck.erase(deck.begin());
_echo(players, ch->getSock(), "{} comprou uma carta.", ch->getName());
if(args!="-s")
{
ch->showCards();
}
}

void Table::jumpChange(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use pular_vez para passar sua vez de jogar a outro jogador.");
return;
}
if(acumulator.size()>0)
{
ch->print("N�o � poss�vel fazer isso agora.");
return;
}
_echo(players, 0, "{} passou a vez!", ch->getName());
this->next_player();
_echo(players, 0, "� a vez de {} jogar.", current_player->getName());
current_player->showCards();
if(current_player->isBot())
{
dlb::dlb_event_send(150, this->getId(), "");
}
}

void Table::cmd_bot(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use o comando bot para gerenciar os bots na partida.");
ch->print("\"bot add <nome>\", Adiciona um novo bot na partida.");
ch->print("bot remove <nome>\" remove o bot especificado da partida.");
return;
}
if(ch->isBot())
{
return;
}
std::string arg1="", arg2="";
StringUtils sc;
sc.parse(args, arg1, arg2);
if(arg1=="add")
{
if(arg2.size()<4)
{
ch->print("Erro, o nome precisa ter pelo menos 4 caracteres.");
return;
}
shared_player bh=std::make_shared<Bot>();
bh->setName(arg2);
this->add_player(bh);
}
else if(arg1=="remove")
{
this->remove_player(arg2);
}
}

//Fun��es...
uint32 table_generate_id()
{
static uint32 x=0;
x++;
return x;
}

void t_send_to_list(std::vector<shared_player>& cons, uint32 ignore_sock, const std::string& msg)
{
for(auto& it: cons)
{
if((it->isBot())||(ignore_sock==it->getSock()))
{
continue;
}
it->print(msg);
}
}


#endif
