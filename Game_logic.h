#ifndef Game_logicH
#define Game_logicH

#include <ctime>
#include <cstdlib>
#include <vector>

static const int S = 10;

class Game_logic {
private:
    // FIX: прибрано inline-ініціалізатори полів (= false, = -1, = 0).
    // C++Builder до XE7 не підтримує ініціалізацію не-static полів у тілі класу.
    // Всі значення ініціалізуються у конструкторі Game_logic().
    bool is_hunting;
    bool axis_fixed;
    int  first_hit_x, first_hit_y;
    int  last_hit_x,  last_hit_y;
    int  current_dir; // 0:вгору 1:вправо 2:вниз 3:вліво

    bool is_valid_coord(int x, int y) {
        return (x >= 0 && x < S && y >= 0 && y < S);
    }

    int random_shot(int field[S][S]);

public:
    int my_field[S][S];
    int attacking_field[S][S];

    Game_logic();
    ~Game_logic() {}

    // Управління полем
    void setup(int field[S][S]);
    void clear(int field[S][S]);
    void all_clean();

    // Скидає стан бота і attacking_field (my_field не чіпає)
    void reset_hunt();

    int  getCellValue(int x, int y, int field[S][S]);
    bool has_ships(int field[S][S]);

    // Розміщення
    bool Cell_is_free(int x, int y, int len, int dir, int field[S][S]);
    void Add_ship(int len, int field[S][S]);

    // Постріл
    int  make_shot(int x, int y, int field[S][S]);
    bool is_destroyed(int x, int y, int field[S][S]);
    void mark_around_destroyed(int x, int y, int field[S][S]);

    // Бот
    int bot_move(int field[S][S]);
};

#endif
