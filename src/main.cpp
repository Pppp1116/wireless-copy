#include "header.hpp"


Message message;


int main(int argc, char** argv) {
    if (argc < 1) {
        return 0;
    }

    std::string option = argv[1];

    if (option == "listen") {
        while (true) {
            std::cout << "port and ip to listen on\n";
            auto words = Listen::where();
            auto message_received = Listen::hear(words);
            auto result = Listen::clipsend(message_received.content);
            if (!result) {
                perror("error writint");
            }
        }
    }
    if (option == "whisper"){
        auto words =Listen::where();
        
        Whisper::clipread();
        auto result = Whisper::send(words);
        if(!result){
            perror("error  sending");
        }
    }
}
