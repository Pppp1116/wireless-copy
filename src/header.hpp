#include <asio.hpp>
#include <boost/asio.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>

struct Message {
    std::string content;
};

extern Message message;

namespace Listen {
std::vector<std::string> where();

Message hear(std::vector<std::string>& words);
[[nodiscard]] bool clipsend(const std::string& text);
} // namespace Listen

namespace Whisper {
[[nodiscard]] bool send(std::vector<std::string>& words);
void clipread() ;
} // namespace Whisper
