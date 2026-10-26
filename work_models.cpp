#include <cstddef>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <omp.h>

using Values = std::vector<int>;
using Ownership = std::vector<int>; // One thread/worker ID recorded per input index.

int transform(int value) { return value * value; }

void transformSequential(const Values& input, Values& output)
{
    for (std::size_t i = 0; i < input.size(); ++i)
        output[i] = transform(input[i]);
}

void transformWithJthreads(const Values& input, Values& output,
                           Ownership& ownerByIndex, int workers)
{
    // TODO 3. Create exactly 'workers' std::jthread objects and assign each a
    // different contiguous part of [0, input.size()). For worker w, use:
    //   begin = n * w / workers;
    //   end   = n * (w + 1) / workers.
    // For each processed index i, write BOTH:
    //   output[i] = transform(input[i]);
    //   ownerByIndex[i] = w;
    // Each index must be processed once. Join all workers before returning.
    (void)input;
    (void)output;
    (void)ownerByIndex;
    (void)workers;
    throw std::logic_error("TODO 3: implement std::jthread work partitioning");
}

void transformWithOpenMP(const Values& input, Values& output,
                         Ownership& ownerByIndex, int workers)
{
    // TODO 4. Use #pragma omp parallel for and request 'workers' threads.
    // For each independent iteration i, compute the same output as the
    // sequential function AND record the participating OpenMP thread ID:
    //   ownerByIndex[index] = omp_get_thread_num();
    // Use a signed loop bound and cast the index to std::size_t as needed.
    (void)input;
    (void)output;
    (void)ownerByIndex;
    (void)workers;
    throw std::logic_error("TODO 4: implement OpenMP loop worksharing");
}

void verifyValues(const Values& expected, const Values& actual, const char* label)
{
    if (expected != actual)
        throw std::runtime_error(std::string(label) + " produced an incorrect result");
}

void verifyOwners(const Ownership& owners, int requestedWorkers, const char* label)
{
    for (std::size_t i = 0; i < owners.size(); ++i) {
        if (owners[i] < 0 || owners[i] >= requestedWorkers)
            throw std::runtime_error(std::string(label) +
                                     " did not record a valid worker for index " +
                                     std::to_string(i));
    }
}

void displayDistribution(const Ownership& owners, int requestedWorkers,
                         const char* implementation, const char* workerLabel)
{
    std::cout << "  " << implementation << ":\n";
    for (int worker = 0; worker < requestedWorkers; ++worker) {
        std::cout << "    " << workerLabel << ' ' << worker << ":";
        int count = 0;
        for (std::size_t i = 0; i < owners.size(); ++i) {
            if (owners[i] == worker) {
                std::cout << ' ' << i;
                ++count;
            }
        }
        if (count == 0) std::cout << " (no iterations recorded)";
        std::cout << "  [" << count << " elements]\n";
    }
}

struct Options {
    int workers = 3;
    bool showDistribution = false;
};

Options parseOptions(int argc, char* argv[])
{
    Options options;
    bool countGiven = false;
    for (int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
        const std::string argument = argv[argumentIndex];
        if (argument == "--trace") {
            if (options.showDistribution)
                throw std::invalid_argument("Specify --trace only once.");
            options.showDistribution = true;
            continue;
        }
        if (countGiven)
            throw std::invalid_argument("Usage: work_models [1..17] [--trace]");

        std::size_t consumed = 0;
        const int requested = std::stoi(argument, &consumed);
        if (consumed != argument.size() || requested < 1 || requested > 17)
            throw std::invalid_argument("Worker count must be an integer from 1 to 17.");
        options.workers = requested;
        countGiven = true;
    }
    return options;
}

int main(int argc, char* argv[])
{
    try {
        const auto options = parseOptions(argc, argv);
        constexpr std::size_t elementCount = 17;

        Values input(elementCount);
        std::iota(input.begin(), input.end(), 1);
        Values expected(elementCount);
        Values withJthreads(elementCount);
        Values withOpenMP(elementCount);
        Ownership jthreadOwners(elementCount, -1);
        Ownership openmpOwners(elementCount, -1);

        transformSequential(input, expected);
        transformWithJthreads(input, withJthreads, jthreadOwners, options.workers);
        transformWithOpenMP(input, withOpenMP, openmpOwners, options.workers);

        verifyValues(expected, withJthreads, "std::jthread");
        verifyValues(expected, withOpenMP, "OpenMP");
        verifyOwners(jthreadOwners, options.workers, "std::jthread");
        verifyOwners(openmpOwners, options.workers, "OpenMP");

        std::cout << "Requested worker count: " << options.workers << '\n';
        std::cout << "Correctness: both implementations match the sequential reference.\n";
        std::cout << "Results: ";
        for (const int result : expected) std::cout << result << ' ';
        std::cout << '\n';

        if (options.showDistribution) {
            std::cout << "Index ownership (diagnostic mode, not a performance test):\n";
            displayDistribution(jthreadOwners, options.workers, "std::jthread", "worker");
            displayDistribution(openmpOwners, options.workers, "OpenMP", "thread");
            std::cout << "OpenMP iteration assignment depends on the runtime and configuration.\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
