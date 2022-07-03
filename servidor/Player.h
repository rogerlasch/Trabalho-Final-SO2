



#ifndef PLAYER_H
#define PLAYER_H

#ifndef TABLE_H
class Table;
typedef std::shared_ptr<Table> shared_table;
#endif

enum player_flags
{
player_expectator=(1<<0),//Ele é expectador...
player_playing=(1<<1),//Ele está jogando ou irá jogar...
player_admin=(1<<2),
player_turn=(1<<3),
player_waiting_uno=(1<<4),
player_uno=(1<<5)
};

enum player_type
{
player_normal=0,
player_bot
};

class Player : public basic_connection, public dlb::dlb_basic_flags
{
protected:
int id;
uint32 type;
int64 unotime;
std::string name;
Deck cards;
shared_table table;
mutable std::shared_mutex mtx;
public:
Player();
Player(const Player& p)=delete;
Player& operator=(const Player& p)=delete;
~Player();
uint32 getType()const;
uint32 deckSize()const;
bool isBot()const;
bool isPlayer()const;
void setId(int id);
int getId()const;
void setName(const std::string& name);
std::string getName()const;
void setUnoTime(int64 utime);
int64 getUnoTime()const;
void setDeck(const Deck& d);
Deck getDeck()const;
void setTable(const shared_table& t);
shared_table getTable()const;
void add_card(const shared_card& c);
shared_card remove_card(uint32 index);
shared_card get_card(uint32 index);
void showCards();
void dropCards();
void check_uno();
};
typedef std::shared_ptr<Player> shared_player;

#endif

#ifndef PLAYER_IMPLEMENTATION
#define PLAYER_IMPLEMENTATION

Player::Player()
{
this->type=player_normal;
this->replace_flags(0);
this->setUnoTime(gettimestamp());
}

Player::~Player()
{
}

uint32 Player::getType()const
{
return this->type;
}

uint32 Player::deckSize()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->cards.size();
}

bool Player::isBot()const
{
return this->type==player_bot;
}

bool Player::isPlayer()const
{
return this->type==player_normal;
}

void Player::setId(int id)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->id=id;
}

int Player::getId()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->id;
}

void Player::setName(const std::string& name)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->name=name;
}

std::string Player::getName()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->name;
}

void Player::setUnoTime(int64 utime)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->unotime=utime;
}

int64 Player::getUnoTime()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->unotime;
}

void Player::setDeck(const Deck& d)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->cards=d;
}

Deck Player::getDeck()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->cards;
}

void Player::setTable(const shared_table& t)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->table=t;
}

shared_table Player::getTable()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->table;
}

void Player::add_card(const shared_card& c)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
cards.push_back(c);
}

shared_card Player::remove_card(uint32 index)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
if(index>cards.size()-1)
{
return shared_card();
}
shared_card c=cards[index];
cards.erase(cards.begin()+index);
return c;
}

shared_card Player::get_card(uint32 index)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
if(index>cards.size()-1)
{
return shared_card();
}
return cards[index];
}

void Player::showCards()
{
if(isBot())
{
return;
}
std::stringstream ss;
ss<<"Sua mão contém um total de "<<cards.size()<<" cartas"<<std::endl;
for(uint32 i=0; i<cards.size(); i++)
{
ss<<fmt::format("({}): {} ", (i+1), cards[i]->toString());
if(i<cards.size()-1)
{
ss<<", ";
}
}
ss<<std::endl;
this->print(ss.str());
}

void Player::dropCards()
{
cards.clear();
}

void Player::check_uno()
{
if(this->deckSize()<2)
{
if(!this->flag_contains(player_waiting_uno))
{
this->setflag(player_waiting_uno);
this->setUnoTime(gettimestamp());
}
}
else
{
this->removeflag(player_waiting_uno);
this->removeflag(player_uno);
}
}
#endif
