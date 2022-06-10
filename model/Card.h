/**
* Escrito por Roger Lasch e Bruno Butka.
* Definição da classe Cards.
**/

#ifndef CARD_H
#define CARD_H

enum Card_types
{
normal=1;
block=2;
reverse=3;
plus_two=4;
plus_four=5;
change_color=6;    
};

enum Card_color
{
black=1;
red=2;
green=3;
yellow=4;
blue=5;
};

class Card
{
private:
int number;
int color;
int type; 
public:
std::string toString()const;
}

typedef std::shared_ptr<Card> shared_card;

#endif

#ifndef CARDS_IMPLEMENTATION_H
#define CARDS_IMPLEMENTATION_H

std::string Card::toString() const
{
return "|Número: " + std::to_string(number) + " | Cor: " + std::to_string(number) + " | Tipo: " + std::to_string(type);
}

#endif
