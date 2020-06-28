#include <cmath>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <iomanip>
#include <future>
#include <exception>
#include <stdint.h>

std::mutex mtx;

bool isPrime(uint64_t x){
    if (x <= 1) return false;

    for(uint64_t i = 2; i * i <= x; ++i)
        if (!(x % i)) return false;

    return true;
}

void action(uint64_t low, uint64_t high, std::vector<std::promise<uint64_t>>& p) {
    size_t numPrimes = 0;
    uint64_t i;

    std::lock_guard<std::mutex> guard(mtx);
    try {
        for (i = low; i <= high; ++i) {
            if (isPrime(i)) {
                p[numPrimes].set_value(i);
                numPrimes++;
                if (numPrimes == p.size()) return;
            }
        }
        if (numPrimes != p.size()) {
            throw high;
        }
    } catch(uint64_t) {
        p[numPrimes].set_exception(std::current_exception());
    }
    return;
}

int main(void) {
    uint64_t low, high;
    int32_t count;

    std::cin >> low >> high >> count;
    if (low >= high) return 1;
    if (count <= 0) return 1;

    int32_t number_of_primes = 0;
    std::vector<std::promise<uint64_t>> p(count);

    std::thread t(action, low, high, std::ref(p));

    try {
        while (number_of_primes < count) {
            std::future <uint64_t> value = p[number_of_primes].get_future();
            number_of_primes++;
            std::cout << value.get() << std::endl;
        }
    } catch (uint64_t e) {
        std::cout << e << std::endl;
    }

    t.join();
    return 0;
}
