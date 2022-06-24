


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

class Table
{
private;
uint32 id;
std::atomic<uint32> current_turn;
std::atomic<uint32> gstate;
std::atomic<uint32> pindex;
std::atomic<uint32> turn_dir;
std::vector<shared_player> players;
Deck deck;
Deck discard;
Deck acumulator;
shared_player current_player;
shared_card current_card;
public:
Table();
Table(const Table& t)=delete;
Table& operator=(const Table& t);
~Table();
shared_player find_player(uint32 sock);
shared_player find_player(const std::string& name);
uint32 get_gstate()const;
void generate_cards();
bool start_game();
void process_command(uint32 sock, const std::string& cmdline);
private:
void swap_turn();
shared_player next_player(bool jump_one=false);
};

void send_to_all_in_table(std::vector<shared_player>& players, const std::string& msg);


#endif
