#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <vector>

static int NUMBER_OF_THREADS = 3;
static int NUMBER_OF_OPERATIONS = 1000000;

std::mutex mtx;

void oper(int id, double add, double sub, std::vector<double>& arr) {
    for (int i = 0; i < NUMBER_OF_OPERATIONS; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        switch (id) {
            case 0:
                arr[id] += add;
                arr[id + 1] -= sub;
                break;
            case 1:
                arr[id] += add;
                arr[id + 1] -= sub;
                break;
            case 2:
                arr[id] += add;
                arr[id % id] -= sub;
        }
    }
    return;
}

int main(void) {
    std::vector<std::thread> ourThreads(NUMBER_OF_THREADS);
    std::vector<double> arr(NUMBER_OF_THREADS, 0);

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        double add = (i * 20) + 80;
        double sub = (i * 20) + 90;
        ourThreads[i] = std::thread(oper, i, add, sub, std::ref(arr));
    }

    for (auto& th : ourThreads) {
        th.join();
    }

    for (auto i : arr) {
        printf("%.10g\n", i);
    }

    return 0;
}
