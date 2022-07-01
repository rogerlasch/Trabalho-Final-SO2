/**
* Escrito por Roger Lasch e Bruno Butka.
* Defini√ß√£o da classe Cards.
**/

#ifndef CARD_H
#define CARD_H

enum Card_types
{
normal=1,
block=2,
reverse_turn=3,
plus_two=4,
plus_four=5,
joker=6
};

enum Card_color
{
uncolor=0,
black=1,
red=2,
green=3,
yellow=4,
blue=5
};

class Card
{
private:
int number;
int color;
int type;
mutable std::shared_mutex mtx;
public:
Card();
Card(const Card& c)=delete;
Card& operator=(const Card& c);
void setNumber(int n);
int getNumber()const;
void setType(int type);
int getType()const;
void setColor(int c);
int getColor()const;
std::string toString()const;
};
typedef std::shared_ptr<Card> shared_card;
typedef std::vector<shared_card> Deck;

std::string color_to_string(int c);
#endif

#ifndef CARDS_IMPLEMENTATION_H
#define CARDS_IMPLEMENTATION_H

Card::Card()
{
setNumber(0);
setType(normal);
setColor(uncolor);
}

void Card::setNumber(int n)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->number=n;
}

int Card::getNumber()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->number;
}

void Card::setType(int type)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->type=type;
}

int Card::getType()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->type;
}

void Card::setColor(int c)
{
std::unique_lock<std::shared_mutex> lck(this->mtx);
this->color=c;
}

int Card::getColor()const
{
std::shared_lock<std::shared_mutex> lck(this->mtx);
return this->color;
}

std::string Card::toString()const
{
std::stringstream ss;
switch(this->getType())
{
case normal:
{
ss<<fmt::format("{} {}", getNumber(), color_to_string(getColor()));
break;
}
case block:
{
ss<<fmt::format("Pular {}", color_to_string(getColor()));
break;
}
case reverse_turn:
{
ss<<fmt::format("Inverter {}", color_to_string(getColor()));
break;
}
case plus_two:
{
ss<<fmt::format("+2 {}", color_to_string(getColor()));
break;
}
case plus_four:
{
ss<<"+4";
break;
}
case joker:
{
ss<<"Coringa";
break;
}
default:
{
ss<<fmt::format("Carta desconhecida! N˙mero: {} Cor: {}, Tipo: {}", getNumber(), getColor(), getType());
break;
}
}
return ss.str();
}

//FunÁıes que n„o fazem parte da classe.

inline std::string color_to_string(int c)
{
    static std::map<int, std::string> color_table={
{uncolor, "Neutro"},
{black, "Preto"},
{red, "Vermelho"},
{green, "Verde"},
{yellow, "Amarelo"},
{blue, "Azul"}
    };
auto it=color_table.find(c);
return ((it==color_table.end()) ? "" : it->second);
}
#endif
