#include "header.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

// listens port
Message hear(std::vector<std::string>& words) {
    try {
        int port = std::stoi(words[1]);

        asio::io_context io;

        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), (asio::ip::port_type)port);
        asio::ip::tcp::acceptor acceptor(io, endpoint);

        asio::ip::tcp::socket socket(io);

        acceptor.accept(socket); // wait for connection

        asio::read(socket, asio::buffer(&message, sizeof(message)));

        // now message is fully received
        return message;
    } catch (const std::exception& e) {
        std::cout << "not number\n";
        exit(1);
    }
}

// asks port
std::vector<std::string> where() {
    std::vector<std::string> words;
    words.reserve(2);

    std::string word;

    std::cout << "port number?\n";
    std::string line;
    std::getline(std::cin, line);

    std::istringstream iss(line);

    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}

[[nodiscard]] bool clipsend(const std::string& text) {
    const char* cmd = std::getenv("WAYLAND_DISPLAY") ? "wl-copy" : "xclip -selection clipboard";

    FILE* pipe = popen(cmd, "w");
    if (pipe == nullptr) {
        return false;
    }

    size_t written = fwrite(text.data(), 1, text.size(), pipe);
    int status = pclose(pipe);

    return written == text.size() && status == 0;
}

void clipread() {
    const char* cmd =
        std::getenv("WAYLAND_DISPLAY") ? "wl-paste -n" : "xclip -selection clipboard -o";

    FILE* pipe = popen(cmd, "r");
    if (pipe == nullptr) {
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        message.content += buffer;
    }

    pclose(pipe);
}