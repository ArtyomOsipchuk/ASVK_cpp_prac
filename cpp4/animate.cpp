#include <bits/stdc++.h>
#include <ncurses.h>
#include <unistd.h>

using namespace std;

static const int WIDTH  = 50;
static const int HEIGHT = 50;
static const int GENOME_SIZE = WIDTH * HEIGHT;
static const int LIFE_STEPS = 100;

inline int idx(int x, int y) {
    return y * WIDTH + x;
}

class ConwayLife {
public:
    static void step(const vector<uint8_t> &current, vector<uint8_t> &next) {
        if ((int)next.size() != GENOME_SIZE)
            next.assign(GENOME_SIZE, 0);
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                int alive = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT) continue;
                        if (current[idx(nx, ny)]) ++alive;
                    }
                }
                bool curAlive = current[idx(x, y)] != 0;
                bool newAlive = false;
                if (curAlive) {
                    newAlive = (alive == 2 || alive == 3);
                } else {
                    newAlive = (alive == 3);
                }
                next[idx(x, y)] = newAlive ? 1 : 0;
            }
        }
    }
};

bool load_matrix(const string &filename, vector<uint8_t> &genome) {
    ifstream in(filename);
    if (!in) {
        cerr << "Cannot open file: " << filename << "\n";
        return false;
    }
    genome.assign(GENOME_SIZE, 0);
    string line;
    int y = 0;
    while (y < HEIGHT && std::getline(in, line)) {
        if ((int)line.size() < WIDTH) {
            cerr << "Line too short in file: " << filename << "\n";
            return false;
        }
        for (int x = 0; x < WIDTH; ++x) {
            char c = line[x];
            genome[idx(x, y)] = (c == 'X' || c == 'x') ? 1 : 0;
        }
        ++y;
    }
    if (y != HEIGHT) {
        cerr << "Not enough lines in file: " << filename << "\n";
        return false;
    }
    return true;
}

void draw_state_ncurses(const vector<uint8_t> &genome, int offset_y = 0, int offset_x = 0) {
    for (int y = 0; y < HEIGHT; ++y) {
        move(offset_y + y, offset_x);
        for (int x = 0; x < WIDTH; ++x) {
            char c = genome[idx(x, y)] ? 'X' : '-';
            addch(c);
        }
    }
    refresh();
}


int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <solution_file>\n";
        cerr << "Example: " << argv[0] << " series_0_run_1_sol.txt\n";
        return 1;
    }

    string filename = argv[1];
    vector<uint8_t> current;
    if (!load_matrix(filename, current)) {
        return 1;
    }

    vector<uint8_t> next(GENOME_SIZE);

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    if (max_y < HEIGHT || max_x < WIDTH) {
        endwin();
        cerr << "Terminal too small for 50x50 field.\n";
        return 1;
    }

    int steps = LIFE_STEPS;
    for (int step_num = 0; step_num <= steps; ++step_num) {
        clear();
        mvprintw(0, 0, "File: %s  Step: %d / %d   (press 'q' to quit)", filename.c_str(), step_num, steps);
        draw_state_ncurses(current, 1, 0);

        for (int i = 0; i < 50; ++i) {
            int ch = getch();
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
            usleep(10000);
        }

        if (step_num == steps) break;
        ConwayLife::step(current, next);
        current.swap(next);
    }

    nodelay(stdscr, FALSE);
    mvprintw(HEIGHT + 2, 0, "Finished. Press any key to exit.");
    refresh();
    getch();

    endwin();
    return 0;
}
