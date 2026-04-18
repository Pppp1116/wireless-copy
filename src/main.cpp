#include "chrono"
#include "header.hpp"
#include <thread>

Message message;
File file;

int main(int argc, char** argv) {
    std::string option;

    if (argc < 2) {
    begin:
        std::cout << "select option 1) listen clipboard or 2) whisper clipboard 3) listen file "
                     "4)whisper file \n";
        std::cin >> option;
        if (option != "1" && option != "2" && option != "3" && option != "4") {
            std::cout << "wrong option\n";
            goto begin;
        }
    } else {
        option = argv[1];
    }

    if (option == "listen" || option == "1") {
        auto words = Listen::where_to_hear();

        while (true) {
            auto message_received = Listen::hear(words);
            if (message_received.clipboard_content.empty()) {
                continue;
            }
            auto result = Listen::clipsend(message_received.clipboard_content);
            if (!result) {
                perror("error writint");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    if (option == "whisper" || option == "2") {
        auto words = Whisper::where_to_whisper();

        while (true) {
            Whisper::clipread();
            auto result = Whisper::send(words);
            if (!result) {
                perror("error  sending");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    if (option == "3") {
        std::string path;
        std::cout << "whats the file path\n";
        std::cin >> path;
        Listen::file_receiver(path);
        goto begin;
    }
    if (option == "4") {
        std::string path;
        std::cout << "whats the file path\n";
        std::cin >> path;
        Whisper::file_sender(path);
        goto begin;
    }
}
