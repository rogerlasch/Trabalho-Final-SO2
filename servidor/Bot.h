

#ifndef BOT_H
#define BOT_H

// Define as decisões do bot
enum BOT_ACTIONS
{
    b_play = 0, // Jogar
    b_buy,      // Comprar carta
    b_jump      // Pular
};

// Converte o tipo de comando retornado para um comando a ser processado posteriormente
struct bot_args
{
    uint32 cmd_type;
    int32 index;
    uint32 color_id;

    // Analisa as propriedades e retorna uma string a ser interpretada pelo interpretador de comandos
    std::string toString()
    {
        std::stringstream ss;
        switch (cmd_type)
        {
            case b_play: // Traduz o comando para uma string "jogar cor"
            {
                ss << "jogar " << (index + 1);
                if ((color_id >= red) && (color_id <= blue))
                {
                    ss << " " << color_to_string(color_id);
                }
                break;
            }

            case b_buy: // Converte o comando para uma string de pesca
            {
                ss << "pescar -s";
                break;
            }

            case b_jump: // Converte o comando para uma string de pular a vez
            {
                ss << "pular_vez";
            }
        }
        return ss.str();
    }
};

// Herda as funcionalidades do player
class Bot : public Player
{
private:
    uint32 fished; // Quantas vezes no turno o bot pescou
    uint32 max_fish; // Quantas vezes pode pescar antes de passar a vez

public:
    Bot();
    Bot(const Bot &b) = delete;
    Bot &operator=(const Bot &b) = delete;
    virtual ~Bot();
    uint32 decision(const shared_card &c, bot_args *args);
    uint32 select_color();
    virtual void print(const std::string &str);
    friend class Table;
};
typedef std::shared_ptr<Bot> shared_bot;

#endif

#ifndef BOT_IMPLEMENTATION
#define BOT_IMPLEMENTATION

// Construtor da classe
Bot::Bot()
    : Player()
{
    this->type = player_bot;
    this->fished = 0;
    this->max_fish = 0;
}

// Destruidor da classe
Bot::~Bot()
{
}

// Toma a decisao de que carta sera jogada
uint32 Bot::decision(const shared_card &c, bot_args *args)
{
    CardFinder sf;

    switch (c->getType())
    {
        case plus_two: // Caso a ultima carta jogada for um +2 ou +4
        case plus_four:
        {
            // Diz se tiver um +2 ou +4 acumulando, ele deve obrigatoriamente jogar uma dessas duas cartas na mão
            if (this->getTable()->calculate_plus_cards() > 0) 
            {
                args->cmd_type = b_play;
                args->index = sf.find_card_type(cards, {plus_two, plus_four});
                
                // Verifica se a carta encontrada é um +4, se sim, seleciona uma cor
                if ((args->index > -1) && (cards[args->index]->getType() == plus_four))
                {
                    args->color_id = select_color();
                }

                return args->cmd_type;
            }

            args->cmd_type = b_play;

            // Tenta encontrar uma carta da mesma cor, caso não encontre, tenta encontrar encontrar um coringa ou +4
            if (((args->index = sf.find_card_color(cards, {c->getColor()})) > -1) || ((args->index = sf.find_card_type(cards, {joker, plus_four})) > -1))
            {
                uint32 ctype = cards[args->index]->getType();

                // Se for um +4 ou coringa, escolhe uma cor
                if ((ctype == plus_four) || (ctype == joker))
                {
                    args->color_id = select_color();
                }
                return args->cmd_type;
            }
            args->cmd_type = b_buy;

            return args->cmd_type;
        }

        case joker:
        {
            args->cmd_type = b_play;

            // Procura da mesma cor ou enta encontra um coringa ou +4
            if (((args->index = sf.find_card_color(cards, {c->getColor()})) > -1) || ((args->index = sf.find_card_type(cards, {joker, plus_four})) > -1))
            {
                uint32 ctype = cards[args->index]->getType();

                // Se for um coringa ou +4, escolhe uma cor, senão, pesca
                if ((ctype == joker) || (ctype == plus_four))
                {
                    args->color_id = select_color();
                }
                return args->cmd_type;
            }
            args->cmd_type = b_buy;
            return args->cmd_type;
        }

        // Caso seja uma carta bloqueante, reverso ou normal
        case block:
        case reverse_turn:
        case normal:
        {
            args->cmd_type = b_play;

            // Verifica se a ultima carta jogada foi uma normal
            if (c->getType() == normal)
            {
                uint32 color = c->getColor();

                // Tenta procurar um pular, inverter e +2 da mesma cor da carta jogada
                if ((args->index = sf.find_TypeColor(cards, {{reverse_turn, color}, {block, color}, {plus_two, color}})) > -1)
                {
                    return args->cmd_type;
                }

                // Se nao encontrar, procura uma carta ou com a mesma cor ou numero da ultima carta jogada
                if (((args->index = sf.find_card_color(cards, {c->getColor()})) > -1) || ((args->index = sf.find_card_number(cards, {c->getNumber()})) > -1))
                {
                    return args->cmd_type;
                }

                // Se nao encontrar, tenta buscar um coringa ou +$
                if ((args->index = sf.find_card_type(cards, {joker, plus_four})) > -1)
                {
                    args->color_id = select_color();
                    return args->cmd_type;
                }

                // Se passar por tudo, comprar
                args->cmd_type = b_buy;
                
                return args->cmd_type;
            } // Tenta encontrar uma carta ou do mesmo tipo ou da mesma cor para jogar (quando for jogada uma carta especial)
            else if (((args->index = sf.find_card_type(cards, {c->getType()})) > -1) || ((args->index = sf.find_card_color(cards, {c->getColor()})) > -1))
            {
                return args->cmd_type;
            }
            else
            {
                 // Ultima tentativa, procurando um coringa ou +4
                if ((args->index = sf.find_card_type(cards, {plus_four, joker})) > -1)
                {
                    args->color_id = select_color();
                    return args->cmd_type;
                }
                
                // Se nada der certo, compra
                args->cmd_type = b_buy;
                return args->cmd_type;
            }
            
            // Se nada der certo, compra
            args->cmd_type = b_buy;
            return args->cmd_type;
        }
    }

    // Caso comprou e nao deu certo, pula a vez
    args->cmd_type = b_jump;
    return args->cmd_type;
}

// Metodo para escolher uma cor com base na quantidade de cartas que possui na mao para a jogada do coringa ou +4
uint32 Bot::select_color()
{
    std::map<uint32, uint32> colors;

    // Conta quantas cartas de cada cor tem
    for (auto &it : cards)
    {
        uint32 c = it->getColor();

        if ((c >= red) && (c <= blue))
        {
            colors[c] += 1;
        }
    }

    // Se tiver apenas uma carta colorida na mao
    if (colors.size() > 0)
    {
        auto higher = colors.begin();

        for (auto it = colors.begin(); it != colors.end(); ++it)
        {
            if (it->second > higher->second)
            {
                higher = it;
            }
        
        }
        if (higher != colors.end())
        {
            return higher->first;
        }
    }

    // Caso nao tenha nenhuma carta colorida, escolhe uma aleatoriamente
    return random_int32(red, blue);
}

void Bot::print(const std::string &str)
{
    _log(str);
}
#endif
