


#ifndef BOT_H
#define BOT_H

enum BOT_ACTIONS
{
b_play=0,
b_buy,
b_jump
};

struct bot_args
{
uint32 cmd_type;
int32 index;
uint32 color_id;
std::string toString()
{
std::stringstream ss;
switch(cmd_type)
{
case b_play:
{
ss<<"jogar "<<(index+1);
if((color_id>=red)&&(color_id<=blue))
{
ss<<" "<<color_to_string(color_id);
}
break;
}
case b_buy:
{
ss<<"pescar -s";
break;
}
case b_jump:
{
ss<<"pular_vez";
}
}
return ss.str();
}
};

class Bot : public Player
{
public:
Bot();
Bot(const Bot& b)=delete;
Bot& operator=(const Bot& b)=delete;
virtual ~Bot();
uint32 decision(const shared_card& c, bot_args* args);
uint32 select_color();
virtual void print(const std::string& str);
};
typedef std::shared_ptr<Bot> shared_bot;

#endif

#ifndef BOT_IMPLEMENTATION
#define BOT_IMPLEMENTATION

Bot::Bot()
: Player()
{
this->type=player_bot;
}

Bot::~Bot()
{
}

uint32 Bot::decision(const shared_card& c, bot_args* args)
{
CardFinder sf;
/*
if(cards.size()>1)
{
std::random_device rd;
std::mt19937 g(rd());
std::shuffle(cards.begin(), cards.end(), g);
}
*/
switch(c->getType())
{
case plus_two:
case plus_four:
{
if(this->getTable()->calculate_plus_cards()>0)
{
args->cmd_type=b_play;
args->index=sf.find_card_type(cards, {plus_two, plus_four});
if((args->index>-1)&&(cards[args->index]->getType()==plus_four))
{
args->color_id=select_color();
}
return args->cmd_type;
}
args->cmd_type=b_play;
if((args->index=sf.find_card_color(cards, {c->getColor()}))==-1)
{
args->cmd_type=b_buy;
}
return args->cmd_type;
break;
}
case joker:
{
args->cmd_type=b_play;
if((args->index=sf.find_card_color(cards, {c->getColor()}))==-1)
{
if((args->index=sf.find_card_type(cards, {joker}))==-1)
{
args->cmd_type=b_buy;
}
else
{
args->color_id=this->select_color();
}
}
else
{
args->cmd_type=b_buy;
}
return args->cmd_type;
}
case block:
case reverse_turn:
case normal:
{
args->cmd_type=b_play;
if(c->getType()==normal)
{
if(((args->index=sf.find_card_color(cards, {c->getColor()}))>-1)||((args->index=sf.find_card_number(cards, {c->getNumber()}))>-1))
{
return args->cmd_type;
}
uint32 color=c->getColor();
if((args->index=sf.find_TypeColor(cards, {{reverse_turn, color}, {block, color}, {plus_two, color}}))>-1)
{
return args->cmd_type;
}
args->cmd_type=b_buy;
return args->cmd_type;
}
else if(((args->index=sf.find_card_type(cards, {c->getType()}))>-1)||((args->index=sf.find_card_color(cards, {c->getColor()}))>-1))
{
return args->cmd_type;
}
else
{
if((args->index=sf.find_card_type(cards, {plus_four, joker}))>-1)
{
args->color_id=select_color();
return args->cmd_type;
}
args->cmd_type=b_buy;
return args->cmd_type;
}
args->cmd_type=b_buy;
return args->cmd_type;
}
}
args->cmd_type=b_jump;
return args->cmd_type;
}

uint32 Bot::select_color()
{
std::map<uint32, uint32> colors;
for(auto& it: cards)
{
uint32 c=it->getColor();
if((c>=red)&&(c<=blue))
{
colors[c]+=1;
}
}
if(colors.size()>0)
{
auto higher=colors.begin();
for(auto it=colors.begin(); it!=colors.end(); ++it)
{
if(it->second>higher->second)
{
higher=it;
}
}
if(higher!=colors.end())
{
return higher->first;
}
}
return random_int32(red, blue);
}

void Bot::print(const std::string& str)
{
_log(str);
}
#endif
