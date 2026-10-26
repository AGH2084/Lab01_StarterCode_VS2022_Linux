#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <omp.h>

struct ThreadObservation {
    int id;
    int teamSize;
    bool activeParallelRegion;
};

struct TeamObservation {
    std::vector<ThreadObservation> arrivalOrder;
};

std::optional<int> parseRequestedThreads(int argc, char* argv[])
{
    if (argc == 1) return std::nullopt;
    if (argc != 2) throw std::invalid_argument("Usage: omp_team_explorer [positive-thread-count]");

    const std::string argument = argv[1];
    std::size_t used = 0;
    const int requested = std::stoi(argument, &used);
    if (used != argument.size() || requested <= 0)
        throw std::invalid_argument("Thread count must be a positive integer.");
    return requested;
}

void printRuntimeContext()
{
    std::cout << "Outside a parallel region:\n";
    std::cout << "  current team size     : " << omp_get_num_threads() << '\n';

    // TODO 1. Replace each placeholder with an OpenMP runtime query
    // supported by Visual Studio 2022 (/openmp) and GCC.
    // Distinguish the number of available processors from the maximum team size
    // reported for a subsequent parallel region.
    std::cout << "  available processors  : " << "<complete>" << '\n';
    std::cout << "  max threads (next)    : " << "<complete>" << '\n';
    std::cout << "  dynamic teams enabled : " << "<complete>" << '\n';
    std::cout << "  in active parallelism : " << "<complete>" << '\n';
}

TeamObservation observeTeam()
{
    TeamObservation observation;

    #pragma omp parallel
    {
        const int id = omp_get_thread_num();

        // TODO 2. Obtain the actual size of the current team and determine
        // whether this execution context is inside an active parallel region.
        // A one-thread parallel region need not be active for omp_in_parallel().
        const int teamSize = -1;
        const bool active = false;

        // Each entire record is appended without a concurrent vector mutation.
        #pragma omp critical(team_log)
        {
            observation.arrivalOrder.push_back({id, teamSize, active});
        }
    }
    return observation;
}

void displayTeam(const TeamObservation& observed)
{
    std::cout << "Recorded threads: " << observed.arrivalOrder.size() << '\n';
    std::cout << "Arrival order at the critical section (not a scheduling guarantee): ";
    for (const auto& row : observed.arrivalOrder) std::cout << row.id << ' ';
    std::cout << '\n';

    auto byId = observed.arrivalOrder;
    std::sort(byId.begin(), byId.end(),
              [](const auto& left, const auto& right) { return left.id < right.id; });

    std::cout << "Thread records sorted by team-local ID:\n";
    for (const auto& row : byId) {
        std::cout << "  id=" << row.id << ", team=" << row.teamSize
                  << ", active=" << std::boolalpha << row.activeParallelRegion << '\n';
    }
}

int main(int argc, char* argv[])
{
    try {
        const auto requested = parseRequestedThreads(argc, argv);
        if (requested) omp_set_num_threads(*requested);

        std::cout << "Requested threads: ";
        if (requested) std::cout << *requested << '\n';
        else std::cout << "runtime default\n";

        printRuntimeContext();
        displayTeam(observeTeam());
        std::cout << "After region: team size=" << omp_get_num_threads() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
