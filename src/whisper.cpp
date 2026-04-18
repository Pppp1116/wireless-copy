#include "header.hpp"

namespace Whisper {

namespace {
asio::ip::port_type parse_port(const std::string& value) {
    int parsed_port = 0;
    try {
        parsed_port = std::stoi(value);
    } catch (const std::exception&) {
        throw std::runtime_error("invalid port");
    }

    if (parsed_port < 1 || parsed_port > 65535) {
        throw std::runtime_error("invalid port");
    }

    return static_cast<asio::ip::port_type>(parsed_port);
}
} // namespace

std::vector<std::string> where_to_whisper() {
    std::vector<std::string> words;
    words.reserve(2);

    std::string line;
    std::string word;

    std::cout << "port number and ip separated by a space " << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, line);

    std::istringstream iss(line);
    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}

[[nodiscard]] bool send(std::vector<std::string>& words) {
    if (words.size() < 2) {
        std::cerr << "missing host or port\n";
        return false;
    }

    try {
        asio::io_context io;
        asio::ssl::context ctx(asio::ssl::context::tls_client);
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(asio::ssl::verify_none);

        asio::ip::tcp::resolver resolver(io);
        asio::ssl::stream<asio::ip::tcp::socket> stream(io, ctx);

        auto const port = parse_port(words[1]);
        auto const endpoints = resolver.resolve(words[0], std::to_string(port));

        asio::connect(stream.next_layer(), endpoints);
        stream.handshake(asio::ssl::stream_base::client);

        std::string payload = message.clipboard_content;
        payload.push_back('\0');
        asio::write(stream, asio::buffer(payload));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "network error: " << e.what() << '\n';
        return false;
    }
}

void clipread() {
    message.clipboard_content.clear();

    const char* cmd = read_clipboard();
    if (cmd == nullptr) {
        std::cerr << "clipboard read is not supported on this platform\n";
        return;
    }

    FILE* pipe = popen(cmd, "r");
    if (pipe == nullptr) {
        perror("clipboard read failed");
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        message.clipboard_content += buffer;
    }

    pclose(pipe);
}
int file_sender(const std::filesystem::path& path) {
    (void)path;
    std::cerr << "file send is not implemented yet\n";
    return 0;
}

} // namespace Whisper

namespace Compatability {
 constexpr uint64_t host_to_big_u64(uint16_t x) {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(x);
    } else {
        return x;
    }
}
constexpr uint64_t big_to_host(uint64_t x) {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(x);

    } else {
        return x;
    }
}
} // namespace Compatability
