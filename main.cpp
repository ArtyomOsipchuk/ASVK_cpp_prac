#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

const char *SOCKET_PATH = "/tmp/sasocket";

class Solution {
public:
    virtual void applyMutation(class MutationOperator&) = 0;
    virtual double getCost() const = 0;
    virtual Solution* clone() const = 0;
    virtual ~Solution() {}
};

class MutationOperator {
public:
    virtual void mutate(Solution& sol) = 0;
    virtual ~MutationOperator() {}
};

class CoolingSchedule {
public:
    virtual double getNextTemperature(double currTemp, int iter) const = 0;
    virtual ~CoolingSchedule() {}
};

class BoltzmannCooling : public CoolingSchedule {
public:
    double getNextTemperature(double currTemp, int iter) const override {
        return currTemp / std::log(1 + iter);
    }
};

class CauchyCooling : public CoolingSchedule {
    double beta;
public:
    double getNextTemperature(double currTemp, int iter) const override {
        return currTemp / (1 + iter);
    }
};

class LinearCooling : public CoolingSchedule {
    double gamma;
public:
    double getNextTemperature(double currTemp, int iter) const override {
        return currTemp * 0.99;
    }
};

struct Job { int id; int duration; };

class Schedule: public Solution {
public:
    int N, M;
    std::vector<std::vector<int>> processors; // каждый процессор хранит номера работ
    std::vector<int> jobTimes;

    Schedule(int M, const std::vector<int>& times, const std::vector<std::vector<int>>& processors)
    : M(M), N(times.size()), jobTimes(times), processors(processors) {}

    Schedule(int M, const std::vector<int>& times)
        : N(times.size()), M(M), jobTimes(times) {
        processors.resize(M);
        std::vector<int> jobs(N);
        for(int i=0; i<N; i++) jobs[i]=i;
        std::shuffle(jobs.begin(), jobs.end(), std::mt19937(time(nullptr)));
        for(int i=0; i<N; i++)
            processors[i % M].push_back(jobs[i]);
    }

    double getCost() const override {
        double total = 0.0;
        for(int proc = 0; proc < M; ++proc) {
            int currTime = 0;
            for(int jobIdx : processors[proc]) {
                currTime += jobTimes[jobIdx];
                total += currTime;
            }
        }
        return total;
    }

    void applyMutation(MutationOperator& mut) override {
        mut.mutate(*this);
    }
    Solution* clone() const override { return new Schedule(*this); }
    void print() const {
        for(int p = 0; p < M; ++p) {
            std::cout << "Process " << p << ": ";
            int t = 0;
            for(int job: processors[p]) {
                t += jobTimes[job];
                std::cout << "(job="<< job << " Time=" << t << ") ";
            }
            std::cout << "\n";
        }
    }

    void swapJobsRandom() {
        if (M < 2) return;
        std::mt19937 gen(time(nullptr) + rand());
        int p1 = gen() % M, p2 = gen() % M;
        while (p2 == p1) p2 = gen() % M;
        if (processors[p1].empty() || processors[p2].empty()) return;
        int i1 = gen() % processors[p1].size();
        int i2 = gen() % processors[p2].size();
        std::swap(processors[p1][i1], processors[p2][i2]);
    }
};

class SwapMutation : public MutationOperator {
public:
    void mutate(Solution& s) override {
        auto& sch = dynamic_cast<Schedule&>(s);
        sch.swapJobsRandom();
    }
};

class SimulatedAnnealing {
    Solution* current;
    Solution* best;
    MutationOperator* mutator;
    CoolingSchedule* cooler;
    double temp;
    int maxNoImprove;
    std::ofstream logFile;
public:
    SimulatedAnnealing(Solution* initial, MutationOperator* m, CoolingSchedule* c, double startTemp)
        : current(initial), best(initial->clone()), mutator(m), cooler(c), temp(startTemp), maxNoImprove(100), logFile("sa_log.csv") {
        logFile << "Iteration,Temperature,CurrentCost,BestCost\n";
    }

    void run() {
        int iter = 0, noImprove = 0;
        while (noImprove < maxNoImprove) {
            Solution* candidate = current->clone();
            //std::cout << "До мутации:\n";
            //dynamic_cast<Schedule*>(candidate)->print();
            mutator->mutate(*candidate);
            //std::cout << "После мутации:\n";
            //dynamic_cast<Schedule*>(candidate)->print();
            double dF = candidate->getCost() - current->getCost();
            if (dF <= 0 || exp(-dF/temp) > 0) {
                delete current;
                current = candidate;
                if (current->getCost() < best->getCost()) {
                    delete best;
                    best = current->clone();
                    noImprove = 0;
                } else {
                    noImprove++;
                }
            } else {
                delete candidate;
                noImprove++;
            }
            logFile << iter << "," << temp << "," << current->getCost() << "," << best->getCost() << "\n";
            temp = cooler->getNextTemperature(temp, iter);
            iter++;
        }
    }
    Solution* getBest() const { return best; }
};

void send_schedule(int sock, const Schedule &s) {
    int M = s.processors.size();
    write(sock, &M, sizeof(M));
    int N = s.jobTimes.size();
    write(sock, &N, sizeof(N));
    write(sock, s.jobTimes.data(), N * sizeof(int));
    for(int i=0; i<M; i++) {
        int nJobs = s.processors[i].size();
        write(sock, &nJobs, sizeof(nJobs));
        write(sock, s.processors[i].data(), nJobs * sizeof(int));
    }
}

Schedule recv_schedule(int sock) {
    int M, N;
    read(sock, &M, sizeof(M));
    read(sock, &N, sizeof(N));
    std::vector<int> jobTimes(N);
    read(sock, jobTimes.data(), N * sizeof(int));
    std::vector<std::vector<int>> processors(M);
    for(int i=0; i<M; i++) {
        int nJobs;
        read(sock, &nJobs, sizeof(nJobs));
        processors[i].resize(nJobs);
        read(sock, processors[i].data(), nJobs * sizeof(int));
    }
    return Schedule(M, jobTimes, processors);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./main <Nproc> <M> <cooling: B C L>\n";
        return 1;
    }
    int Nproc = std::atoi(argv[1]);
    int M = std::atoi(argv[2]);
    char cooling = argv[3][0];
    if (cooling == 'L') {
        std::cout << "Linear cooling\n";
    } else if (cooling == 'B') {
        std::cout << "Boltzmann cooling\n";
    } else if (cooling == 'C') {
        std::cout << "Cauchy cooling\n";
    } else {
        std::cout << "ERROR: Bad cooling\n";
        return 1;
    }
    int N = 0;
    double startTemp = 100.0;
    std::vector<int> jobDurations;
    std::ifstream in("jobs.csv");
    if (!in.is_open()) {
        std::cerr << "Ошибка открытия файла jobs.csv" << std::endl;
        return 1;
    }

    std::string line;
    std::getline(in, line);
    std::getline(in, line);
    while (std::getline(in, line)) {
        std::stringstream s(line);
        std::string jobIdStr, durationStr;
        if (std::getline(s, jobIdStr, ',') && std::getline(s, durationStr)) {
                int duration = std::stoi(durationStr);
                jobDurations.push_back(duration);
                N++;
        }
    }
    in.close();

    int listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH);
    bind(listen_sock, (sockaddr*)&addr, sizeof(addr));
    listen(listen_sock, Nproc);

    auto t0=clock();
    for(int i=0; i < Nproc; i++) {
        if(fork() == 0) {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            sockaddr_un caddr = {};
            caddr.sun_family = AF_UNIX;
            strcpy(caddr.sun_path, SOCKET_PATH);
            connect(sock, (sockaddr*)&caddr, sizeof(caddr));
            Schedule start = recv_schedule(sock);

            for (int sync_iter = 0; sync_iter < 10; ++sync_iter) {
                SwapMutation mutator;
                double startTemp = 100.0;
                if (cooling == 'L') {
                    LinearCooling cooler;
                    SimulatedAnnealing sa(new Schedule(start), &mutator, &cooler, startTemp);
                    sa.run();
                    Schedule* best = dynamic_cast<Schedule*>(sa.getBest());
                    send_schedule(sock, *best);
                    delete best;
                    start = recv_schedule(sock);
                } else if (cooling == 'B') {
                    BoltzmannCooling cooler;
                    SimulatedAnnealing sa(new Schedule(start), &mutator, &cooler, startTemp);
                    sa.run();
                    Schedule* best = dynamic_cast<Schedule*>(sa.getBest());
                    send_schedule(sock, *best);
                    delete best;
                    start = recv_schedule(sock);
                } else {
                    CauchyCooling cooler;
                    SimulatedAnnealing sa(new Schedule(start), &mutator, &cooler, startTemp);
                    sa.run();
                    Schedule* best = dynamic_cast<Schedule*>(sa.getBest());
                    send_schedule(sock, *best);
                    delete best;
                    start = recv_schedule(sock);
                }
            }
            close(sock);
            exit(0);
        }
    }

    std::vector<int> conns;
    for(int i=0; i < Nproc; i++)
        conns.push_back(accept(listen_sock, NULL, NULL));

    Schedule initial(M, jobDurations);
    for(int i=0; i < Nproc; i++)
        send_schedule(conns[i], initial);

    for (int sync_iter = 0; sync_iter < 10; sync_iter++) {
        std::vector<Schedule> workerSchedules;
        std::vector<double> costs;
        for(int i=0; i < Nproc; i++) {
            Schedule ws = recv_schedule(conns[i]);
            workerSchedules.push_back(ws);
            costs.push_back(ws.getCost());
        }
        int best_idx = std::min_element(costs.begin(), costs.end()) - costs.begin();
        Schedule globalBest = workerSchedules[best_idx];
        //std::cout << "Синхронизация " << sync_iter << " Best K2: " << globalBest.getCost() << std::endl;
        if (sync_iter == 9) {
            std::cout << "Best K2: " << globalBest.getCost() << std::endl;
        }
        for(int i=0;i < Nproc;i++) send_schedule(conns[i], globalBest);
    }
    for(int i=0; i < Nproc; i++) close(conns[i]);
    close(listen_sock);
    unlink(SOCKET_PATH);
    auto dt=(clock()-t0)/CLOCKS_PER_SEC;
    //std::cout << "Time:" << dt << "\n";
    return 0;
}
