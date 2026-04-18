#include <iostream>
#include <fstream>
#include "header.hpp"
#include <chrono>
#include <iomanip>
#include <concepts>

#define DEBUG_PATH "applogs/debug.log"

namespace DEBUG {

template<writable T>
void log(const T& text) {
    std::ofstream file(DEBUG_PATH, std::ios::app);

    if (!file) {
        perror("error logging");
        return;
    }

    // get time
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    // write
    file << std::setw(10) << text
         << " | "
         << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
         << '\n';
}


}
