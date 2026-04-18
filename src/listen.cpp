#include "header.hpp"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace Listen {

namespace {
std::vector<std::string> read_words(const std::string& prompt) {
    std::vector<std::string> words;
    words.reserve(2);

    std::string line;
    std::string word;

    std::cout << prompt << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, line);

    std::istringstream iss(line);
    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}
} // namespace

// listens port
Message hear(asio::ip::port_type listen_port) {
    try {
        Message received_message{};

        asio::io_context io;

        asio::ssl::context ctx(asio::ssl::context::tls_server);

        ctx.use_certificate_chain_file("certs/server.crt");
        ctx.use_private_key_file("certs/server.key", asio::ssl::context::pem);

        

        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), listen_port);
        asio::ip::tcp::acceptor acceptor(io, endpoint);

        asio::ssl::stream<asio::ip::tcp::socket> stream(io, ctx);

        std::cout << "listening on port "
                  << acceptor.local_endpoint().port()
                  << '\n';

        acceptor.accept(stream.next_layer());

        stream.handshake(asio::ssl::stream_base::server);

        asio::streambuf payload;
        asio::read_until(stream, payload, '\0');

        std::istream payload_stream(&payload);
        std::getline(payload_stream, received_message.clipboard_content, '\0');

        return received_message;
    } catch (const std::exception& e) {
        std::cerr << "hear() failed: " << e.what() << '\n';
        return {};
    }
}

std::vector<std::string> where_to_hear() {
    return read_words("port number ");
}

[[nodiscard]] bool clipsend(const std::string& text) {
    const char* cmd = std::getenv("WAYLAND_DISPLAY") ? "wl-copy" : "xclip -selection clipboard";

    FILE* pipe = popen(cmd, "w");
    if (pipe == nullptr) {
        return false;
    }

    size_t const written = fwrite(text.data(), 1, text.size(), pipe);
    int const status = pclose(pipe);


    return written == text.size() && status == 0;
}
int file_receiver(const std::filesystem::path& output_dir) {
    (void)output_dir;
    std::cerr << "file receive is not implemented yet\n";
    return 0;
}

} // namespace Listen
