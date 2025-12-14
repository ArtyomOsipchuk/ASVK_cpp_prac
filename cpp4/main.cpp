#include <bits/stdc++.h>
using namespace std;

static const int WIDTH  = 50;
static const int HEIGHT = 50;
static const int GENOME_SIZE = WIDTH * HEIGHT;
static const int N_POP = 100;
static const int LIFE_STEPS = 100;
static const int MAX_NO_IMPROVE = 50;
static const double INIT_ONE_PROB = 0.75;
static const double CROSS_PROB = 0.8;
static const double PENALTY = 1e6;


inline int idx(int x, int y) {
    return y * WIDTH + x;
}


class ConwayLife {
public:
    static void step(const vector<uint8_t> &current, vector<uint8_t> &next) {
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

    static double evaluate(const vector<uint8_t> &start, bool &is_stationary, vector<uint8_t> *out_after100 = nullptr) {
        vector<uint8_t> cur = start;
        vector<uint8_t> next(GENOME_SIZE);
        for (int i = 0; i < LIFE_STEPS; ++i) {
            step(cur, next);
            cur.swap(next);
        }
        if (out_after100) *out_after100 = cur;
        step(cur, next);
        is_stationary = true;
        for (int i = 0; i < GENOME_SIZE; ++i) {
            if (cur[i] != next[i]) {
                is_stationary = false;
                break;
            }
        }
        int aliveCount = 0;
        for (int i = 0; i < GENOME_SIZE; ++i)
            if (cur[i]) ++aliveCount;
        return static_cast<double>(aliveCount);
    }
};


struct Individual {
    vector<uint8_t> genome;
    double fitness;
    Individual() : genome(GENOME_SIZE, 0), fitness(numeric_limits<double>::infinity()) {}
};


class FitnessEvaluator {
public:
    virtual ~FitnessEvaluator() = default;
    virtual double evaluate(const Individual &ind) = 0;
};

class SelectionOperator {
public:
    virtual ~SelectionOperator() = default;
    virtual int select_one(const vector<Individual> &pop) = 0;
};

class CrossoverOperator {
public:
    virtual ~CrossoverOperator() = default;
    virtual void crossover(const Individual &p1, const Individual &p2,
                           Individual &c1, Individual &c2) = 0;
};

class MutationOperator {
public:
    virtual ~MutationOperator() = default;
    virtual void mutate(Individual &ind) = 0;
};


class LifeFitness : public FitnessEvaluator {
public:
    double evaluate(const Individual &ind) override {
        bool is_stationary = false;
        double base = ConwayLife::evaluate(ind.genome, is_stationary, nullptr);
        if (is_stationary) {
            return base + PENALTY;
        }
        return base;
    }
};


class TournamentSelection : public SelectionOperator {
    int tournament_size;
    mt19937 &rng;
public:
    TournamentSelection(int k, mt19937 &r) : tournament_size(k), rng(r) {}
    int select_one(const vector<Individual> &pop) override {
        uniform_int_distribution<int> dist(0, (int)pop.size() - 1);
        int best = dist(rng);
        double bestFit = pop[best].fitness;
        for (int i = 1; i < tournament_size; ++i) {
            int idx = dist(rng);
            if (pop[idx].fitness < bestFit) {
                best = idx;
                bestFit = pop[idx].fitness;
            }
        }
        return best;
    }
};


class TwoPointCrossover : public CrossoverOperator {
    mt19937 &rng;
public:
    TwoPointCrossover(mt19937 &r) : rng(r) {}
    void crossover(const Individual &p1, const Individual &p2,
                   Individual &c1, Individual &c2) override {
        uniform_int_distribution<int> dist(0, GENOME_SIZE - 1);
        int a = dist(rng);
        int b = dist(rng);
        if (a > b) swap(a, b);
        c1.genome = p1.genome;
        c2.genome = p2.genome;
        for (int i = a; i <= b; ++i) {
            c1.genome[i] = p2.genome[i];
            c2.genome[i] = p1.genome[i];
        }
    }
};

class BitFlipMutation : public MutationOperator {
    mt19937 &rng;
    double p_mut;
public:
    BitFlipMutation(mt19937 &r, double p) : rng(r), p_mut(p) {}
    void mutate(Individual &ind) override {
        uniform_real_distribution<double> dist(0.0, 1.0);
        for (int i = 0; i < GENOME_SIZE; ++i) {
            if (dist(rng) < p_mut) {
                ind.genome[i] ^= 1;
            }
        }
    }
};


class GeneticAlgorithm {
    mt19937 &rng;
    unique_ptr<FitnessEvaluator> fitness;
    unique_ptr<SelectionOperator> selection;
    unique_ptr<CrossoverOperator> crossover;
    unique_ptr<MutationOperator> mutation;

    vector<Individual> population;
    Individual best;
    int no_improve;

public:
    GeneticAlgorithm(mt19937 &r,
                     unique_ptr<FitnessEvaluator> f,
                     unique_ptr<SelectionOperator> s,
                     unique_ptr<CrossoverOperator> c,
                     unique_ptr<MutationOperator> m)
        : rng(r), fitness(move(f)), selection(move(s)), crossover(move(c)), mutation(move(m)),
          population(N_POP), no_improve(0) {
        best.fitness = numeric_limits<double>::infinity();
    }

    void init_population() {
        uniform_real_distribution<double> dist(0.0, 1.0);
        for (int j = 0; j < N_POP; ++j) {
            for (int i = 0; i < GENOME_SIZE; ++i) {
                population[j].genome[i] = (dist(rng) < INIT_ONE_PROB) ? 1 : 0;
            }
        }
    }

    void evaluate_population() {
        for (int i = 0; i < N_POP; ++i) {
            population[i].fitness = fitness->evaluate(population[i]);
            if (population[i].fitness < best.fitness) {
                best = population[i];
                no_improve = 0;
            }
        }
    }

    void evolve() {
        init_population();
        evaluate_population();
        no_improve = 0;

        while (no_improve < MAX_NO_IMPROVE) {
            vector<Individual> new_pop(N_POP);
            uniform_real_distribution<double> dist01(0.0, 1.0);

            for (int i = 0; i < N_POP; i += 2) {
                int p1_idx = selection->select_one(population);
                int p2_idx = selection->select_one(population);
                Individual child1, child2;
                if (dist01(rng) < CROSS_PROB) {
                    crossover->crossover(population[p1_idx], population[p2_idx], child1, child2);
                } else {
                    child1.genome = population[p1_idx].genome;
                    child2.genome = population[p2_idx].genome;
                }
                mutation->mutate(child1);
                mutation->mutate(child2);
                new_pop[i] = move(child1);
                if (i + 1 < N_POP) new_pop[i + 1] = move(child2);
            }

            population.swap(new_pop);
            double oldBest = best.fitness;
            evaluate_population();
            if (best.fitness < oldBest)
                no_improve = 0;
            else
                ++no_improve;
        }
    }

    const Individual &get_best() const { return best; }
};


void save_matrix(const string &filename, const vector<uint8_t> &genome) {
    ofstream out(filename);
    if (!out) return;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            out << (genome[idx(x, y)] ? 'X' : '-');
        }
        out << '\n';
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <series_i> <run_id>\n";
        return 1;
    }
    int series_i = atoi(argv[1]);
    int run_id   = atoi(argv[2]);

    double Pmut_init = 0.0004;
    double pmut = Pmut_init;
    for (int k = 0; k < series_i; ++k) pmut *= 1.5;

    random_device rd;
    unsigned seed = rd() ^ (unsigned)chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 rng(seed);

    auto start = chrono::high_resolution_clock::now();

    unique_ptr<FitnessEvaluator> fitness(new LifeFitness());
    unique_ptr<SelectionOperator> selection(new TournamentSelection(3, rng));
    unique_ptr<CrossoverOperator> crossover(new TwoPointCrossover(rng));
    unique_ptr<MutationOperator> mutation(new BitFlipMutation(rng, pmut));

    GeneticAlgorithm ga(rng, move(fitness), move(selection), move(crossover), move(mutation));
    ga.evolve();
    const Individual &best = ga.get_best();

    bool dummy_stationary = false;
    vector<uint8_t> after100;
    ConwayLife::evaluate(best.genome, dummy_stationary, &after100);

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = stop - start;
    double elapsed = diff.count();

    ostringstream sol_name, after_name;
    sol_name << "series_" << series_i << "_run_" << run_id << "_sol.txt";
    after_name << "series_" << series_i << "_run_" << run_id << "_sol_after100.txt";

    save_matrix(sol_name.str(), best.genome);
    save_matrix(after_name.str(), after100);

    cout << fixed << setprecision(6) << elapsed << "," << best.fitness << "\n";

    return 0;
}
