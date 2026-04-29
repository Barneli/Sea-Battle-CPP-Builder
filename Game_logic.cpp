#include "Game_logic.h"

// FIX: усі поля, що раніше ініціалізувались у тілі класу (inline),
// тепер ініціалізуються тут — сумісно з усіма версіями C++Builder.
Game_logic::Game_logic() {
    srand(static_cast<unsigned int>(time(0)));

    is_hunting  = false;
    axis_fixed  = false;
    first_hit_x = -1;
    first_hit_y = -1;
    last_hit_x  = -1;
    last_hit_y  = -1;
    current_dir = 0;

    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++) {
            my_field[i][j]        = 0;
            attacking_field[i][j] = 0;
        }
}

void Game_logic::clear(int field[S][S]) {
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            field[i][j] = 0;
}

void Game_logic::reset_hunt() {
    clear(attacking_field);
    is_hunting  = false;
    axis_fixed  = false;
    first_hit_x = last_hit_x = -1;
    first_hit_y = last_hit_y = -1;
    current_dir = 0;
}

void Game_logic::all_clean() {
    clear(my_field);
    clear(attacking_field);
    is_hunting  = false;
    axis_fixed  = false;
    first_hit_x = last_hit_x = -1;
    first_hit_y = last_hit_y = -1;
    current_dir = 0;
}

int Game_logic::getCellValue(int x, int y, int field[S][S]) {
    return field[x][y];
}

bool Game_logic::Cell_is_free(int x, int y, int len, int dir, int field[S][S]) {
    if (dir == 0 && (x + len > S)) return false;
    if (dir == 1 && (y + len > S)) return false;
    for (int i = 0; i < len; i++) {
        int cx = x + (dir == 0 ? i : 0);
        int cy = y + (dir == 1 ? i : 0);
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = cx + dx;
                int ny = cy + dy;
                if (nx >= 0 && nx < S && ny >= 0 && ny < S)
                    if (field[nx][ny] > 0 && field[nx][ny] < 100) return false;
            }
        }
    }
    return true;
}

void Game_logic::Add_ship(int len, int field[S][S]) {
    bool placed   = false;
    int  attempts = 0;
    while (!placed && attempts < 1000) {
        int x   = rand() % S;
        int y   = rand() % S;
        int dir = rand() % 2;
        if (Cell_is_free(x, y, len, dir, field)) {
            int baseIndex = (len == 1) ? 0 : (len == 2) ? 1 : (len == 3) ? 3 : 6;
            if (dir == 1) baseIndex += 10;
            for (int i = 0; i < len; i++) {
                int cx = x + (dir == 0 ? i : 0);
                int cy = y + (dir == 1 ? i : 0);
                field[cx][cy] = baseIndex + i + 1;
            }
            placed = true;
        }
        attempts++;
    }
}

void Game_logic::setup(int field[S][S]) {
    clear(field);
    Add_ship(4, field);
    for (int i = 0; i < 2; i++) Add_ship(3, field);
    for (int i = 0; i < 3; i++) Add_ship(2, field);
    for (int i = 0; i < 4; i++) Add_ship(1, field);
}

bool Game_logic::is_destroyed(int x, int y, int field[S][S]) {
    std::vector<std::pair<int,int> > checked;
    std::vector<std::pair<int,int> > to_process;
    to_process.push_back(std::make_pair(x, y));
    checked.push_back(std::make_pair(x, y));
    int head = 0;
    while (head < (int)to_process.size()) {
        int cx = to_process[head].first;
        int cy = to_process[head].second;
        head++;
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= S || ny < 0 || ny >= S) continue;
            if (field[nx][ny] > 0 && field[nx][ny] < 100) return false;
            if (field[nx][ny] < 0 && field[nx][ny] > -100) {
                bool already = false;
                for (int k = 0; k < (int)checked.size(); k++)
                    if (checked[k].first == nx && checked[k].second == ny) { already = true; break; }
                if (!already) {
                    checked.push_back(std::make_pair(nx, ny));
                    to_process.push_back(std::make_pair(nx, ny));
                }
            }
        }
    }
    return true;
}

void Game_logic::mark_around_destroyed(int x, int y, int field[S][S]) {
    std::vector<std::pair<int,int> > ship_cells;
    ship_cells.push_back(std::make_pair(x, y));
    int head = 0;
    while (head < (int)ship_cells.size()) {
        int cx = ship_cells[head].first;
        int cy = ship_cells[head].second;
        head++;
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= S || ny < 0 || ny >= S) continue;
            if (field[nx][ny] < 0 && field[nx][ny] > -100) {
                bool already = false;
                for (int k = 0; k < (int)ship_cells.size(); k++)
                    if (ship_cells[k].first == nx && ship_cells[k].second == ny) { already = true; break; }
                if (!already) ship_cells.push_back(std::make_pair(nx, ny));
            }
        }
    }
    for (int s = 0; s < (int)ship_cells.size(); s++) {
        for (int ddx = -1; ddx <= 1; ddx++) {
            for (int ddy = -1; ddy <= 1; ddy++) {
                int nx = ship_cells[s].first  + ddx;
                int ny = ship_cells[s].second + ddy;
                if (nx >= 0 && nx < S && ny >= 0 && ny < S && field[nx][ny] == 0)
                    field[nx][ny] = 100;
            }
        }
    }
}

int Game_logic::make_shot(int x, int y, int field[S][S]) {
    if (x < 0 || x >= S || y < 0 || y >= S) return -1;
    if (field[x][y] < 0 || field[x][y] == 100) return -1;
    if (field[x][y] > 0 && field[x][y] < 100) {
        field[x][y] = -field[x][y];
        if (is_destroyed(x, y, field)) {
            mark_around_destroyed(x, y, field);
            return 2;
        }
        return 1;
    }
    field[x][y] = 100;
    return 0;
}

bool Game_logic::has_ships(int field[S][S]) {
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            if (field[i][j] > 0 && field[i][j] < 100) return true;
    return false;
}

int Game_logic::random_shot(int field[S][S]) {
    int safety = 0;
    int x, y;
    do {
        x = rand() % S;
        y = rand() % S;
        if (++safety > 200) return -1;
    } while (field[x][y] < 0 || field[x][y] == 100);

    int res = make_shot(x, y, field);
    if (res == 1) {
        is_hunting  = true;
        axis_fixed  = false;
        first_hit_x = last_hit_x = x;
        first_hit_y = last_hit_y = y;
        current_dir = rand() % 4;
    }
    return res;
}

int Game_logic::bot_move(int field[S][S]) {
    if (!is_hunting)
        return random_shot(field);

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    for (int attempts = 0; attempts < 4; attempts++) {
        int x = last_hit_x + dx[current_dir];
        int y = last_hit_y + dy[current_dir];

        if (!is_valid_coord(x, y) || field[x][y] < 0 || field[x][y] == 100) {
            if (axis_fixed) {
                current_dir = (current_dir + 2) % 4;
                last_hit_x  = first_hit_x;
                last_hit_y  = first_hit_y;
            } else {
                current_dir = (current_dir + 1) % 4;
            }
            continue;
        }

        int res = make_shot(x, y, field);

        if (res == 1) {
            axis_fixed = true;
            last_hit_x = x;
            last_hit_y = y;
            return 1;
        } else if (res == 2) {
            is_hunting = false;
            axis_fixed = false;
            return 2;
        } else if (res == 0) {
            if (axis_fixed) {
                current_dir = (current_dir + 2) % 4;
                last_hit_x  = first_hit_x;
                last_hit_y  = first_hit_y;
            } else {
                current_dir = (current_dir + 1) % 4;
            }
            return 0;
        }
        current_dir = (current_dir + 1) % 4;
    }

    is_hunting = false;
    axis_fixed = false;
    return random_shot(field);
}
