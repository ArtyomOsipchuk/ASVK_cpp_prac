#include <fstream>
#include <random>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./gen <Jobs> <minTime> <maxTime>\n";
        return 1;
    }

    int N = std::atoi(argv[1]);
    int minT = std::atoi(argv[2]);
    int maxT = std::atoi(argv[3]);
    std::mt19937 gen(time(nullptr));
    std::uniform_int_distribution<> dist(minT, maxT);

    std::vector<int> jobDurations;
    for (int i = 0; i < N; ++i) {
        jobDurations.push_back(dist(gen));
        //std::cout << i << " - job, duration: " << jobDurations[i] << "\n";
    }

    std::ofstream out("jobs.csv");
    out << "JobID,Duration\n";
    for(int i=0; i<N; ++i)
        out << i << "," << jobDurations[i] << "\n";
    out.close();
    return 0;
}