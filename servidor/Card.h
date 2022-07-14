/**
 * Escrito por Roger Lasch e Bruno Butka.
 * Definição da classe Cards.
 **/

#ifndef CARD_H
#define CARD_H

// Tipos das cartas
enum Card_types
{
    normal = 1,
    block = 2,
    reverse_turn = 3,
    plus_two = 4,
    plus_four = 5,
    joker = 6
};

// Cores
enum Card_color
{
    uncolor = 0,
    black = 1,
    red = 2,
    green = 3,
    yellow = 4,
    blue = 5
};

class Card
{
private:
    int number; 
    int color;
    int type;
    mutable std::shared_mutex mtx; // Sincroniza os acesso as propriedades

public:
    Card();
    Card(const Card &c) = delete;
    Card &operator=(const Card &c);
    void setNumber(uint32 n);
    uint32 getNumber() const;
    void setType(uint32 type);
    uint32 getType() const;
    void setColor(uint32 c);
    uint32 getColor() const;
    std::string toString() const;
};
typedef std::shared_ptr<Card> shared_card;
typedef std::vector<shared_card> Deck;

std::string color_to_string(int c);
uint32 color_to_int(const std::string &c);

// Estrutura com metodos para verificar o baralho para encontrar uma casta especifica
struct CardFinder
{
    /* Procura no baralho um ou mais tipos de carta
    Ajuda na tomada de decista do bot */
    int32 find_card_type(const Deck &cards, const std::initializer_list<uint32> &types)
    {
        for (uint32 i = 0; i < cards.size(); i++)
        {
            for (auto &it : types)
            {
                if (cards[i]->getType() == it)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    // Procura por uma carta de uma cor especifica
    int32 find_card_color(const Deck &cards, const std::initializer_list<uint32> &colors)
    {
        for (uint32 i = 0; i < cards.size(); i++)
        {
            uint32 c = cards[i]->getColor();
            if ((c < red) || (c > blue))
            {
                continue;
            }
            for (auto &it : colors)
            {
                if (c == it)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    // Procura por uma carta por um numero
    int32 find_card_number(const Deck &cards, const std::initializer_list<uint32> &numbers)
    {
        for (uint32 i = 0; i < cards.size(); i++)
        {
            if (cards[i]->getType() != normal)
            {
                continue;
            }
            for (auto &it : numbers)
            {
                if (it > 9)
                {
                    continue;
                }
                if (cards[i]->getNumber() == it)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    // Procura uma ou mais cartas que correspondam ao tipo E a cor
    uint32 find_TypeColor(const Deck &cards, const std::initializer_list<std::initializer_list<uint32>> &cs)
    {
        for (uint32 i = 0; i < cards.size(); i++)
        {
            int col = cards[i]->getColor();

            if ((col < red) || (col > blue))
            {
                continue;
            }

            for (auto &it : cs)
            {
                std::vector<uint32> c(it);
                switch (c.size())
                {
                case 1:
                {
                    if (cards[i]->getType() == c[0])
                    {
                        return i;
                    }
                    break;
                }

                case 2:
                {
                    if ((cards[i]->getType() == c[0]) && (cards[i]->getColor() == c[1]))
                    {
                        return i;
                    }
                    break;
                }
                }
            }
        }

        return -1;
    }

    // Procura pela cor E pelo numero
    uint32 find_ColorNumber(const Deck &cards, const std::initializer_list<std::initializer_list<uint32>> &cs)
    {
        FuncTimer st(__FUNCTION__);
        for (uint32 i = 0; i < cards.size(); i++)
        {
            if (cards[i]->getType() != normal)
            {
                continue;
            }
            for (auto &it : cs)
            {
                std::vector<uint32> c(it);
                switch (c.size())
                {
                case 1:
                {
                    if (cards[i]->getColor() == c[0])
                    {
                        return i;
                    }
                    break;
                }
                case 2:
                {
                    if ((cards[i]->getColor() == c[0]) && (cards[i]->getNumber() == c[1]))
                    {
                        return i;
                    }
                    break;
                }
                }
            }
        }
        return -1;
    }
};

#endif

#ifndef CARDS_IMPLEMENTATION_H
#define CARDS_IMPLEMENTATION_H

Card::Card()
{
    setNumber(0);
    setType(normal);
    setColor(uncolor);
}

void Card::setNumber(uint32 n)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->number = n;
}

uint32 Card::getNumber() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->number;
}

void Card::setType(uint32 type)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->type = type;
}

uint32 Card::getType() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->type;
}

void Card::setColor(uint32 c)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->color = c;
}

uint32 Card::getColor() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->color;
}

// Analisa o tipo da carta e construi uma descricao para ser mostrada ao usuario
std::string Card::toString() const
{
    std::stringstream ss;
    switch (this->getType())
    {
    case normal:
    {
        ss << fmt::format("{} {}", getNumber(), color_to_string(getColor()));
        break;
    }

    case block:
    {
        ss << fmt::format("Pular {}", color_to_string(getColor()));
        break;
    }

    case reverse_turn:
    {
        ss << fmt::format("Inverter {}", color_to_string(getColor()));
        break;
    }

    case plus_two:
    {
        ss << fmt::format("+2 {}", color_to_string(getColor()));
        break;
    }

    case plus_four:
    {
        ss << "+4";
        uint32 c = this->getColor();
        if ((c >= red) && (c <= blue))
        {
            ss << " " << color_to_string(c);
        }
        break;
    }

    case joker:
    {
        ss << "Coringa";
        uint32 c = this->getColor();
        if ((c >= red) && (c <= blue))
        {
            ss << " " << color_to_string(c);
        }
        break;
    }

    default:
    {
        ss << fmt::format("Carta desconhecida! N�mero: {} Cor: {}, Tipo: {}", getNumber(), getColor(), getType());
        break;
    }
    }

    return ss.str();
}

// Funcoes que nao fazem parte da classe.
// Converte do identificador da cor para cor como string e da string para o identificador
inline std::string color_to_string(int c)
{
    static std::map<int, std::string> color_table = {
        {uncolor, "Neutro"},
        {black, "Preto"},
        {red, "Vermelho"},
        {green, "Verde"},
        {yellow, "Amarelo"},
        {blue, "Azul"}};
    auto it = color_table.find(c);
    return ((it == color_table.end()) ? "" : it->second);
}

// Converte uma string para um int, que identificara a cor que esta pedindo
uint32 color_to_int(const std::string &c)
{
    static std::map<std::string, uint32> colors = {
        {"neutro", uncolor},
        {"preto", black},
        {"vermelho", red},
        {"verde", green},
        {"amarelo", yellow},
        {"azul", blue}};
    StringUtils sc;
    auto it = colors.find(sc.to_lower_case(c));
    return ((it == colors.end()) ? uncolor : it->second);
}
#endif
