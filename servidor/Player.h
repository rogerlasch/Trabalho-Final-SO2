

#ifndef PLAYER_H
#define PLAYER_H

#ifndef TABLE_H
class Table;
typedef std::shared_ptr<Table> shared_table;
#endif

// Diz os comportamentos especiais que o jogador pode ter durante a partida
enum player_flags
{
    player_expectator = (1 << 0), // Ele e expectador...
    player_playing = (1 << 1), // Ele esta jogando ou ira jogar...
    player_admin = (1 << 2), // E quem criou a partida, que manda quando começa o jogo...
    player_turn = (1 << 3), // Significa que e a vez dele jogar
    player_waiting_uno = (1 << 4), // O servidor espera o jogador digitar "UNO"
    player_uno = (1 << 5), // O jogador so tem uma carta na mao e disse "UNO"
    player_fished = (1 << 6) // Ja pescou pelo menos uma carta
};

// Identificar se um "player normal" ou um bot
enum player_type
{
    player_normal = 0,
    player_bot
};

// Classe do player
class Player : public basic_connection, public dlb::dlb_basic_flags
{
protected:
    int id;
    uint32 type;
    uint32 points;
    int64 unotime;
    std::string name;
    Deck cards;
    shared_table table;
    mutable std::shared_mutex mtx;

public:
    Player();
    Player(const Player &p) = delete;
    Player &operator=(const Player &p) = delete;
    ~Player();
    uint32 getType() const;
    uint32 deckSize() const;
    bool isBot() const;
    bool isPlayer() const;
    void setId(int id);
    int getId() const;
    void setPoints(uint32 points);
    uint32 getPoints() const;
    void setName(const std::string &name);
    std::string getName() const;
    void setUnoTime(int64 utime);
    int64 getUnoTime() const;
    void setDeck(const Deck &d);
    Deck getDeck() const;
    void setTable(const shared_table &t);
    shared_table getTable() const;
    void add_card(const shared_card &c);
    shared_card remove_card(uint32 index);
    shared_card get_card(uint32 index);
    void showCards(const shared_card &c = shared_card());
    void dropCards();
    void check_uno();
    void clearScreen();
};
typedef std::shared_ptr<Player> shared_player;

#endif

#ifndef PLAYER_IMPLEMENTATION
#define PLAYER_IMPLEMENTATION

// Construtor
Player::Player()
{
    this->type = player_normal; // Define que ele construiu um objeto de um "player normal" 
    this->setPoints(0); // Inicia a pontuacao do jogador
    this->replace_flags(0); // Inicia os flags do jogador, desativando todas
    this->setUnoTime(gettimestamp());  // Inicia o tempo de espera para o jogador digitar "UNO"
}

// Destruidor
Player::~Player()
{
}

uint32 Player::getType() const
{
    return this->type;
}

// Tamanho do deck do jogador
uint32 Player::deckSize() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->cards.size();
}

// Verifica se o jogador e um bot
bool Player::isBot() const
{
    return this->type == player_bot;
}

// Verifica se o jogador e um player
bool Player::isPlayer() const
{
    return this->type == player_normal;
}

void Player::setId(int id)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->id = id;
}

int Player::getId() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->id;
}

void Player::setPoints(uint32 points)
{
    this->points = points;
}

uint32 Player::getPoints() const
{
    return this->points;
}

void Player::setName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->name = name;
}

std::string Player::getName() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->name;
}

void Player::setUnoTime(int64 utime)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->unotime = utime;
}

int64 Player::getUnoTime() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->unotime;
}

void Player::setDeck(const Deck &d)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->cards = d;
}

Deck Player::getDeck() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->cards;
}

void Player::setTable(const shared_table &t)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    this->table = t;
}

shared_table Player::getTable() const
{
    std::shared_lock<std::shared_mutex> lck(this->mtx);
    return this->table;
}

void Player::add_card(const shared_card &c)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    cards.push_back(c);
}

// Remove a carta indicada pelo indice
shared_card Player::remove_card(uint32 index)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    if (index > cards.size() - 1)
    {
        return shared_card();
    }

    shared_card c = cards[index];
    cards.erase(cards.begin() + index);

    return c;
}

// Retorna uma referencia para a carta que o jogador tem na mao (nao remove, apenas retorna uma referencia)
shared_card Player::get_card(uint32 index)
{
    std::unique_lock<std::shared_mutex> lck(this->mtx);
    if (index > cards.size() - 1)
    {
        return shared_card();
    }
    return cards[index];
}

// Mostrar a ultima carta jogada e as cartas da mao
void Player::showCards(const shared_card &c)
{
    if (isBot())
    {
        return;
    }
    std::stringstream ss;
    if (c != NULL)
    {
        ss << "Última carta jogada: " << c->toString() << std::endl;
    }

    ss << "Sua mão contêm um total de " << cards.size() << " cartas" << std::endl;
    for (uint32 i = 0; i < cards.size(); i++)
    {
        ss << fmt::format("({}): {} ", (i + 1), cards[i]->toString());
        if (i < cards.size() - 1)
        {
            ss << ", ";
        }
    }
    ss << std::endl;
    this->print(ss.str());
}

// Esvazia as cartas da mao
void Player::dropCards()
{
    cards.clear();
}

// Verifica se pode settar a flag de UNO
void Player::check_uno()
{
    if (this->deckSize() < 2)
    {
        if (!this->flag_contains(player_waiting_uno))
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

/* Limpa a tela para deixar melhor a visualização do jogo
Nao foi utilizado pois optamos por deixar o decorrer do jogo */
void Player::clearScreen()
{
    this->print("\b");
}
#endif
