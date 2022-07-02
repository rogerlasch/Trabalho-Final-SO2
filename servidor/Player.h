



#ifndef PLAYER_H
#define PLAYER_H

#ifndef TABLE_H
class Table;
typedef std::shared_ptr<Table> shared_table;
#endif

enum player_game
{
player_default=0,
player_expectator,
player_playing
};

enum player_type
{
player_normal=0,
player_bot
};

class Player : public basic_connection
{
protected:
int id;
uint32 type;
uint32 pstate;
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
bool isBot()const;
bool isPlayer()const;
void setId(int id);
int getId()const;
void setPState(uint32 pstate);
uint32 getPState()const;
void setName(const std::string& name);
std::string getName()const;
void setDeck(const Deck& d);
Deck getDeck()const;
void setTable(const shared_table& t);
shared_table getTable()const;
void add_card(const shared_card& c);
shared_card remove_card(uint32 index);
shared_card get_card(uint32 index);
void showCards();
void dropCards();
};
typedef std::shared_ptr<Player> shared_player;

#endif

#ifndef PLAYER_IMPLEMENTATION
#define PLAYER_IMPLEMENTATION

Player::Player()
{
this->type=player_normal;
}

Player::~Player()
{
}

uint32 Player::getType()const
{
return this->type;
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

void Player::setPState(uint32 pstate)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->pstate=pstate;
}

uint32 Player::getPState()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->pstate;
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
#endif
