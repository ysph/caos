#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <fstream>
#include <thread>

template<class Input, class Output, class Function, class Callback>
void async_vector(const std::vector<Input> ins, Function &&f, Callback &&c) {
    auto lambda = [](const std::vector<Input> ins, Function &&f, Callback &&c) {
        std::vector <Output> result;
        for (size_t i = 0; i < ins.size(); i++) {
            result.push_back(f(ins[i]));
        }
        c(std::forward<decltype(result)>(result));
    };
    std::thread lmao = std::thread(lambda, ins, std::forward<Function>(f), std::forward<Callback>(c));
    lmao.detach();
}

int incr(int a) {
    return a + 1;
}

void print(const std::vector<int> &outs) {
    for (auto v : outs) {
        std::cout << v << std::endl;
    }
}

int main() {
    using namespace std::literals::chrono_literals;

    std::vector<int> ins{1, 2, 3, 4, 5};
    async_vector<int, int>(ins, std::function(incr), std::function(print));
    std::this_thread::sleep_for(500ms);
}
