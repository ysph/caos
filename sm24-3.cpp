#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <future>
#include <iomanip>
#include <climits>
#include <tuple>

std::tuple<unsigned long long, unsigned long long> monte(unsigned id, unsigned long long interval) {
    double rand_x, rand_y;
    unsigned long long origin_dist;
    unsigned long long circle_points = 0;

    for (unsigned long long i = 0; i < interval; i++) {
        rand_x = (double)rand_r(&id) / RAND_MAX * 2.0 - 1;
        rand_y = (double)rand_r(&id) / RAND_MAX * 2.0 - 1;
        origin_dist = rand_x * rand_x + rand_y * rand_y;
        if (origin_dist < 1.0) {
            circle_points++;
        }
    }

    return std::make_tuple(circle_points, interval);
}

int main(int argc, char **argv) {
    unsigned i, threads;
    unsigned long long interval;
    unsigned long long all = 0, inside = 0;
    double pi;

    if (argc == 3) {
        threads = atoi(argv[1]);
        interval = atoi(argv[2]);
    } else {
        return EXIT_FAILURE;
    }
    if (threads < 1 || interval < 1 || interval >= ULLONG_MAX)
        return EXIT_FAILURE;

    std::vector<std::future <std::tuple<unsigned long long, unsigned long long>>> futures(threads);

    for (i = 0; i < threads; i++) {
        futures[i] = std::async(std::launch::async, monte, i, interval);
    }

    for (auto &ft : futures) {
        auto [circleP, squareP] = ft.get();
        inside += circleP;
        all += squareP;
    }
    pi = (double) inside / all * 4.0;

    std::cout << std::fixed << std::setprecision(5) << pi << std::endl;

    return 0;
}
