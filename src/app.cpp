#include "app.hpp"
#include "header.hpp"
#include <chrono>
#include <thread>

namespace {

asio::ip::port_type parse_port_or_throw(const std::vector<std::string>& words) {
    if (words.empty()) {
        throw std::runtime_error("missing port");
    }

    int parsed_port = 0;
    try {
        parsed_port = std::stoi(words.front());
    } catch (const std::exception&) {
        throw std::runtime_error("invalid port");
    }

    if (parsed_port < 1 || parsed_port > 65535) {
        throw std::runtime_error("invalid port");
    }

    return static_cast<asio::ip::port_type>(parsed_port);
}

std::string prompt_for_option() {
    std::string option;

    while (true) {
        std::cout << "select option 1) listen clipboard or 2) whisper clipboard 3) listen file "
                     "4)whisper file \n";
        std::cin >> option;

        if (option == "1" || option == "2" || option == "3" || option == "4") {
            return option;
        }

        std::cout << "wrong option\n";
    }
}

void run_listen_clipboard() {
    auto const listen_port = parse_port_or_throw(Listen::where_to_hear());

    while (true) {
        auto const message_received = Listen::hear(listen_port);
        if (message_received.clipboard_content.empty()) {
            continue;
        }

        if (!Listen::clipsend(message_received.clipboard_content)) {
            perror("error writint");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void run_whisper_clipboard() {
    auto words = Whisper::where_to_whisper();

    while (true) {
        Whisper::clipread();

        if (!Whisper::send(words)) {
            perror("error  sending");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void run_listen_file() {
    std::string path;
    std::cout << "whats the file path\n";
    std::cin >> path;
    Listen::file_receiver(path);
}

void run_whisper_file() {
    std::string path;
    std::cout << "whats the file path\n";
    std::cin >> path;
    Whisper::file_sender(path);
}

} // namespace

Message message;
File file;

namespace App {

int run(int argc, char** argv) {
    std::string option;

    if (argc < 2) {
        option = prompt_for_option();
    } else {
        option = argv[1];
    }

    if (option == "listen" || option == "1") {
        run_listen_clipboard();
    }

    if (option == "whisper" || option == "2") {
        run_whisper_clipboard();
    }

    if (option == "3") {
        run_listen_file();
    }

    if (option == "4") {
        run_whisper_file();
    }

    return 0;
}

} // namespace App
