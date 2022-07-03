


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

enum table_flags
{
table_delay=(1<<0)
};

class Table : public dlb::dlb_basic_flags, public std::enable_shared_from_this<Table>
{
private:
typedef std::function<void(shared_player&, const std::string&)> table_command;
std::map<std::string, table_command> cmdtable;
uint32 id;
std::atomic<uint32> current_turn;
std::atomic<uint32> gstate;
std::atomic<uint32> pindex;
std::atomic<uint32> turn_dir;
std::atomic<uint32> active_players;
std::atomic<int64> delay_start_time;
std::atomic<int64> delay_ms;
std::vector<shared_player> players;
Deck deck;
Deck discard;
Deck acumulator;
shared_player current_player;
shared_card current_card;
std::vector<std::string> winers;
public:
Table();
Table(const Table& t)=delete;
Table& operator=(const Table& t);
~Table();
void setId(uint32 id);
uint32 getId()const;
uint32 playerCount()const;
void make_delay(int64 delay_time);
bool isDelayed();
void table_loop();
std::string toString();
shared_player find_player(uint32 sock);
shared_player find_player(const std::string& name);
shared_player has_admin();
void add_player(const shared_player& c);
void remove_player(const std::string& name);
uint32 get_gstate()const;
void generate_cards();
void process_command(shared_player& ch, const std::string& cmdline);
void interact_bot(shared_player& ch);
uint32 calculate_plus_cards()const;
private:
shared_player next_player();
void internal_PlayCard(shared_player& ch, shared_card& c, uint32 index);
void internal_next_player();
void endGame();
//NÃO INVOCAR OS PRÓXIMOS MÉTODOS DIRETAMENTE!!!!
void commands(shared_player& ch, const std::string& args);
void goBack(shared_player& ch, const std::string& args);
void startGame(shared_player& ch, const std::string& args);
void playCard(shared_player& ch, const std::string& args);
void toFish(shared_player& ch, const std::string& args);
void jumpChange(shared_player& ch, const std::string& args);
void cmd_bot(shared_player& ch, const std::string& args);
void do_who(shared_player& ch, const std::string& args);
void do_chat(shared_player& ch, const std::string& args);
void do_quero(shared_player& ch, const std::string& args);
void do_uno(shared_player& ch, const std::string& args);
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
active_players.store(0);
players.clear();
deck.clear();
discard.clear();
acumulator.clear();
current_player=NULL;
current_card=NULL;
winers.clear();
cmdtable={
{"comandos", std::bind(&Table::commands, this, std::placeholders::_1, std::placeholders::_2)},
{"voltar", std::bind(&Table::goBack, this, std::placeholders::_1, std::placeholders::_2)},
{"startgame", std::bind(&Table::startGame, this, std::placeholders::_1, std::placeholders::_2)},
{"jogar", std::bind(&Table::playCard, this, std::placeholders::_1, std::placeholders::_2)},
{"pescar", std::bind(&Table::toFish, this, std::placeholders::_1, std::placeholders::_2)},
{"pular_vez", std::bind(&Table::jumpChange, this, std::placeholders::_1, std::placeholders::_2)},
{"bot", std::bind(&Table::cmd_bot, this, std::placeholders::_1, std::placeholders::_2)},
{"chat", std::bind(&Table::do_chat, this, std::placeholders::_1, std::placeholders::_2)},
{"who", std::bind(&Table::do_who, this, std::placeholders::_1, std::placeholders::_2)},
{"quero", std::bind(&Table::do_quero, this, std::placeholders::_1, std::placeholders::_2)},
{"uno", std::bind(&Table::do_uno, this, std::placeholders::_1, std::placeholders::_2)},
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

void Table::make_delay(int64 delay_time)
{
if(this->flag_contains(table_delay))
{
return;
}
if(delay_time<=0)
{
return;
}
if(delay_time>5000)
{
delay_time=5000;
}
if((current_player!=NULL)&&(current_player->isBot()))
{
this->setflag(table_delay);
this->delay_start_time.store(gettimestamp());
delay_ms.store(delay_time);
}
}

bool Table::isDelayed()
{
if(this->flag_contains(table_delay))
{
int64 end=gettimestamp();
if((end-delay_start_time.load())>delay_ms.load())
{
this->removeflag(table_delay);
return false;
}
return true;
}
return false;
}

void Table::table_loop()
{
switch(this->get_gstate())
{
case g_starting:
case g_finished:
{
break;
}
case g_playing:
{
if((current_player!=NULL)&&(current_player->isBot())&&(!isDelayed()))
{
interact_bot(current_player);
}
if((current_player!=NULL)&&(current_player->flag_contains(player_waiting_uno)))
{
int64 diff=(gettimestamp()-current_player->getUnoTime());
if(diff>=5000)
{
current_player->removeflag(player_waiting_uno);
_echo(players, 0, "{} esqueceu de anunciar uno, por tanto como penalidade, terá que comprar mais duas cartas!", current_player->getName());
this->process_command(current_player, "pescar -s");
this->process_command(current_player, "pescar -s");
this->internal_next_player();
}
}
}
}
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

shared_player Table::has_admin()
{
shared_player ch;
for(auto& it : players)
{
if(it->flag_contains(player_admin))
{
return it;
}
if((ch==NULL)&&(!it->flag_contains(player_admin)))
{
ch=it;
}
}
if(ch!=NULL)
{
ch->setflag(player_admin);
_echo(players, 0, "Agora {} é o administrador da mesa.", ch->getName());
}
return ch;
}

void Table::add_player(const shared_player& c)
{
if(this->get_gstate()==g_playing)
{
c->setflag(player_expectator);
}
else
{
c->setflag(player_playing);
}
players.push_back(c);
c->setTable(this->shared_from_this());
c->print(fmt::format("Bem-vindo a mesa {} {}.", this->getId(), c->getName()));
if(players.size()==1)
{
c->setflag(player_admin);
c->print("Você agora é o administrador da mesa.");
c->print("Digite: \"startgame\" quando estiver pronto.");
}
else
{
has_admin();
}
if(c->isPlayer())
{
c->print("Digite \"comandos\" para ver uma lista de comandos disponíveis.");
c->print("digite: \"<comando> ?\" para ler a ajuda do comando.");
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
if(ch->flag_contains(player_playing))
{
Deck d=ch->getDeck();
for(auto& it : d)
{
deck.insert(deck.begin()+random_int32(0, deck.size()-1), it);
}
if(pindex.load()==i)
{
next_player();
_echo(players, 0, "É a vez de {} jogar.", current_player->getName());
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
active_players.fetch_sub(1);
}
ch->print("Até logo...");
ch->setTable(shared_table());
players.erase(players.begin()+i);
if(ch->flag_contains(player_admin))
{
this->has_admin();
}
ch->replace_flags(0);
ch->print("Digite comandos para ver uma lista de comandos disponíveis.");
break;
}
}
if(active_players.load()<2)
{
this->endGame();
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
if((players[x]->flag_contains(player_expectator))||(!players[x]->flag_contains(player_playing)))
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
if((players[x-1]->flag_contains(player_expectator))||(!players[x-1]->flag_contains(player_playing)))
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
current_player->removeflag(player_turn);
current_player=ch;
current_player->setflag(player_turn);
pindex.store(x);
}
return ch;
}

void Table::internal_PlayCard(shared_player& ch, shared_card& c, uint32 index)
{
if(this->get_gstate()!=g_playing)
{
return;
}
_echo(players, 0, "{} jogou {}", ch->getName(), c->toString());
_log("Última carta: {}, Carta atual: {}", current_card->toString(), c->toString());
discard.push_back(c);
current_card=discard[discard.size()-1];
ch->remove_card(index);
switch(c->getType())
{
case plus_two:
case plus_four:
{
acumulator.push_back(c);
_echo(players, 0, "Acumulando {}.", c->toString());
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
if(ch->deckSize()==0)
{
_echo(players, 0, "{} terminou o jogo!", ch->getName());
_echo(players, 0, "{} agora é um expectador.", ch->getName());
ch->setflag(player_expectator);
ch->removeflag(player_playing);
ch->removeflag(player_waiting_uno);
ch->removeflag(player_uno);
winers.push_back(ch->getName());
active_players.fetch_sub(1);
if(active_players.load()<2)
{
this->endGame();
return;
}
this->internal_next_player();
return;
}
ch->check_uno();
if((ch->isBot())&&(ch->flag_contains(player_waiting_uno)))
{
this->process_command(ch, "uno");
return;
}
else if(ch->flag_contains(player_waiting_uno))
{
//Retorna porque o servidor deve esperar o jogador digitar uno...
return;
}
this->internal_next_player();
}

void Table::internal_next_player()
{
this->next_player();
_echo(players, 0, "É a vez de {} jogar.", current_player->getName());
current_player->showCards();
if(acumulator.size()>0)
{
CardFinder sf;
int32 x=calculate_plus_cards();
if(sf.find_card_type(current_player->getDeck(), {plus_two, plus_four})==-1)
{
_echo(players, 0, "{} não tem nenhum +2 ou +4, por tanto, terá que comprar {:+} cartas!", current_player->getName(), x);
acumulator.clear();
for(uint32 i=0; i<x; i++)
{
process_command(current_player, "pescar -s");
}
}
}
this->make_delay(2500);
}

void Table::endGame()
{
gstate.store(g_finished);
_echo(players, 0, "O jogo terminou!");
for(auto& it: players)
{
it->dropCards();
it->removeflag(player_uno);
it->removeflag(player_waiting_uno);
this->process_command(it, "quero jogar");
}
std::stringstream ss;
ss<<"Classificação geral dos jogadores..."<<std::endl;
for(uint32 i=0; i<winers.size(); i++)
{
ss<<(i+1)<<": "<<winers[i]<<std::endl;
}
_echo(players, 0, ss.str());
winers.clear();
current_player=shared_player();
current_card=shared_card();
acumulator.resize(0);
deck.resize(0);
active_players.store(0);
pindex.store(0);
gstate.store(g_starting);
shared_player ch=has_admin();
if(ch!=NULL)
{
ch->print("Agora você pode inicciar uma nova partida digitando \"startgame\"");
}
}

void Table::interact_bot(shared_player& ch)
{
FuncTimer sh(__FUNCTION__);
if(ch->isPlayer())
{
return;
}
bool done=false;
shared_bot bh=dynamic_pointer_cast<Bot>(ch);
uint32 result=0;
bot_args args;
result=bh->decision(current_card, &args);
switch(result)
{
case b_play:
{
this->process_command(ch, args.toString());
bh->fished=0;
bh->max_fish=0;
break;
}
case b_jump:
{
this->process_command(ch, args.toString());
break;
}
case b_buy:
{
if(bh->max_fish==0)
{
bh->max_fish=random_int32(1, 3);
}
if(bh->fished<bh->max_fish)
{
bh->fished++;
this->process_command(ch, "pescar -s");
}
else
{
bh->fished=0;
bh->max_fish=random_int32(1, 3);
this->process_command(current_player, "pular_vez");
}
this->make_delay(500);
break;
}
}
}

//Comandos... Não invocar diretamente.
void Table::commands(shared_player& ch, const std::string& args)
{
std::stringstream ss;
ss<<"Comandos disponíveis:"<<std::endl;
for(auto it=cmdtable.begin(); it!=cmdtable.end(); ++it)
{
ss<<it->first<<std::endl;
}
ss<<cmdtable.size()<<" comandos encontrados."<<std::endl;
ss<<"Digite \"<comando> ?\" para mais ajuda sobre um comando."<<std::endl;
ch->print(ss.str());
}

void Table::goBack(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Utilize o comando voltar para sair da mesa e retornar para a sala principal.");
return;
}
remove_player(ch->getName());
}

void Table::startGame(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Digite startgame para iniciar o jogo. Apenas o criador da sala pode iniciar o jogo. Caso o criador saia, o controle passará para o próximo da lista, se possível.");
return;
}
if(this->get_gstate()==g_playing)
{
ch->print("O jogo já começou!");
return;
}
if(!ch->flag_contains(player_admin))
{
ch->print("Apenas o administrador pode iniciar o jogo.");
return;
}
if(players.size()<2)
{
ch->print("Ainda não existem jogadores o suficiente para dar início a partida.");
return;
}
this->gstate.store(g_playing);
shared_player first_player;
uint32 count=0;
for(auto& it : players)
{
if(!it->flag_contains(player_expectator))
{
it->setflag(player_playing);
it->dropCards();
count++;
if(first_player==NULL)
{
first_player=it;
}
}
}
if((first_player==NULL)||(count<2))
{
ch->print("Existem expectadores de mais. O jogo precisa de pelo menos 2 jogadores que não são expectadores para começar.");
this->gstate.store(g_starting);
return;
}
active_players.store(count);
_echo(players, 0, "{} Iniciou o jogo.\nTotal de jogadores na partida: {}", ch->getName(), count);
this->generate_cards();
//Embaralhar as cartas...
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(deck.begin(), deck.end(), g);
for(auto& ch : players)
{
//Expectadores não recebem cartas!
if(ch->flag_contains(player_expectator))
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
current_player=first_player;
current_player->setflag(player_turn);
pindex.store(0);
_echo(players, 0, "A carta virada foi: {}", current_card->toString());
_echo(players, 0, "{} começa o jogo.", current_player->getName());
}

void Table::playCard(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use o comando jogar para jogar uma carta se possível.");
ch->print("Uso: jogar >índice> <cor>");
ch->print("Onde <Índice> é o índice da carta em sua mão. Para saber o índice da carta, verifique o número que aparece entre () antes do nome.");
ch->print("<Cor> é utilizado apenas com cartas especiais como +4 e coringa. As cores aceitas são Verde, Vermelho, Amarelo e Azul.");
ch->print("Exemplos:\n\"jogar 1\" Irá jogar a primeira carta.");
ch->print("\"jogar 2 azul\" Irá jogar a segunda carta, e selecionará a cor azul caso ela seja um +4 ou coringa.");
return;
}
if(this->get_gstate()!=g_playing)
{
ch->print("O jogo ainda não começou!!");
return;
}
if(!ch->flag_contains(player_turn))
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
ch->print("Opção inválida!");
return;
}
shared_card c=ch->get_card(index-1);
if(c==NULL)
{
ch->print("Esta carta não existe em seu baralho.");
return;
}
if(ch->isBot())
{
_log("Jogada de {}, args: {}", ch->getName(), color_str);
_log("Última carta: {} Carta tentada: {}", current_card->toString(), c->toString());
}
if(acumulator.size()>0)
{
CardFinder sf;
if((c->getType()!=plus_two)&&(c->getType()!=plus_four)&&(sf.find_card_type(ch->getDeck(), {plus_two, plus_four})>-1))
{
ch->print(fmt::format("Você não pode jogar esta carta no momento porque um {} está ativo.", current_card->toString()));
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
ch->print("Esta jogada é inválida!");
}
}
else if((c->getType()==plus_four)||(c->getType()==joker))
{
if(color_str.size()==0)
{
ch->print("A cor não foi informada. Repita o comando incluindo uma cor válida como terceiro parâmetro.\nExemplo: \"jogar 1 verde\"");
return;
}
if((color_id<red)||(color_id>blue))
{
ch->print("Esta cor não é válida.");
return;
}
if((current_card->getType()==plus_two)||(current_card->getType()==plus_four))
{
if(c->getType()==joker)
{
ch->print("Esta jogada não é válida!");
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
ch->print("Jogada inválida!");
}
}
else if((c->getType()==current_card->getType())||(c->getColor()==current_card->getColor()))
{
this->internal_PlayCard(ch, c, index-1);
}
else
{
ch->print("Carta inválida!");
}
}

void Table::toFish(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("O comando pescar é utilizado para comprar cartas quando for sua vez de jogar.");
ch->print("Use \"jogar -s\" se não quiser que toda sua mão seja mostrada novamente ao pescar a carta.");
return;
}
if(this->get_gstate()!=g_playing)
{
ch->print("O jogo ainda não começou...");
return;
}
if(!ch->flag_contains(player_turn))
{
ch->print("Espere sua vez!");
return;
}
if(deck.size()==0)
{
_echo(players, 0, "As cartas acabaram! Reembaralhando...");
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
discard.erase(discard.begin(), discard.begin()+x);
//Embaralhar as cartas...
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(deck.begin(), deck.end(), g);
}
ch->add_card(deck[0]);
if(ch->isPlayer())
{
ch->print(fmt::format("Você comprou \"{}\"", deck[0]->toString()));
}
deck.erase(deck.begin());
ch->check_uno();
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
if(this->get_gstate()!=g_playing)
{
ch->print("O jogo ainda não começou...");
return;
}
if(!ch->flag_contains(player_turn))
{
ch->print("Aguarde sua vez antes de desistir!");
return;
}
if(acumulator.size()>0)
{
ch->print("Não é possível fazer isso agora.");
return;
}
_echo(players, 0, "{} passou a vez!", ch->getName());
this->next_player();
_echo(players, 0, "É a vez de {} jogar.", current_player->getName());
current_player->showCards();
this->make_delay(2500);
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
if(this->get_gstate()==g_playing)
{
ch->print("O jogo já começou, o comando ficou indisponível.");
return;
}
if(!ch->flag_contains(player_admin))
{
ch->print("Apenas o administrador pode gerenciar os bots.");
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

void Table::do_who(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use o comando \"who\" para ver quem está atualmente na mesa.");
return;
}
std::stringstream ss;
ss<<"Quem está na mesa?"<<std::endl;
for(auto& it: players)
{
if(it->isBot())
{
ss<<it->getName()<<" (BOT)"<<std::endl;
}
else
{
ss<<it->getName()<<std::endl;
}
}
ss<<players.size()<<" players encontrados."<<std::endl;
ch->print(ss.str());
}

void Table::do_chat(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Use \"<chat> <mensagem>\" para enviar uma mensagem a todos integrantes atualmente na mesa.");
return;
}
std::string str=fmt::format("{} disse \"{}\"", ch->getName(), args);
_echo(players, 0, str);
}

void Table::do_quero(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Este comando lhe permite alternar entre o modo de expectador e de jogador.");
ch->print("Uso: \"quero assistir\" para ser expectador do jogo.");
ch->print("\"quero jogar\" para ser um jogador.");
ch->print("Note que o comando só funcionará antes do jogo ser iniciado.");
return;
}
if(this->get_gstate()==g_playing)
{
ch->print("O jogo já foi iniciado. Não é possível trocar de modo agora!");
return;
}
if(args=="assistir")
{
ch->setflag(player_expectator);
ch->removeflag(player_playing);
ch->print("Agora você é um expectador.");
}
else if(args=="jogar")
{
ch->setflag(player_playing);
ch->removeflag(player_expectator);
ch->print("Agora você é um jogador.");
}
else
{
ch->print("Argumento inválido!");
ch->print("Use \"quero ?\" para obter ajuda.");
}
}

void Table::do_uno(shared_player& ch, const std::string& args)
{
if(args=="?")
{
ch->print("Digite \"uno\" quando estiver com uma carta na mão.");
ch->print("Voc~ê terá 5 segundos para teclar \"uno\" ou será penalizado com a compra de duas cartas.");
return;
}
if(this->get_gstate()!=g_playing)
{
ch->print("O jogo ainda não começou!");
return;
}
if(!ch->flag_contains(player_waiting_uno))
{
if(this->flag_contains(player_uno))
{
ch->print("Você já disse \"UNO\"");
}
else
{
ch->print("Você ainda não está pronto para cantar vitória!");
}
return;
}
ch->setflag(player_uno);
ch->removeflag(player_waiting_uno);
_echo(players, 0, "{} anunciou \"UNO!!!\"", ch->getName());
//
this->internal_next_player();
}

//Funções...
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
