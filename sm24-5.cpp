#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <functional>

template <class F1, class F2, class ...Args>
void async_launch(F1 &&f1, F2 &&f2, Args &&...args) {
    auto fq = [](F1 &&f1, F2 &&f2, Args &&...args) {
        auto res = f1(std::forward<Args>(args)...);

        f2(std::forward<typeof(res)>(res));
    };

    std::thread lmao = std::thread(fq, std::forward<F1>(f1), std::forward<F2>(f2), std::forward<Args>(args)...);
    lmao.detach();
}
