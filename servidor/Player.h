



#ifndef PLAYER_H
#define PLAYER_H


class Player : public basic_connection
{
private;
int id;
std::string name;
Deck cards;
shared_table table;
std::shared_mutex mtx;
public:
Player();
Player(const Player& p)=delete;
Player& operator=(const Player& p)=delete;
~Player();
void setId(int id);
int getId()const;
void setName(const std::string& name);
std::string getName()const;
void setDeck(const Deck& d);
Deck getDeck()const;
void setTable(const shared_table& t);
shared_table getTable()const;
void add_card(const shared_card& c);
shared_card remove_card(int index);
shared_card get_card(int index)
};

#endif

#ifndef PLAYER_IMPLEMENTATION
#define PLAYER_IMPLEMENTATION

Player::Player()
{

}

Player::~Player()
{
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

void add_card(const shared_card& c)
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

#endif
