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

std::vector<std::string> read_host_and_port() {
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
} // namespace

std::vector<std::string> where_to_whisper() {
    return read_host_and_port();
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
    if (!std::filesystem::exists(path)) {
        std::cerr << "file send failed: path does not exist\n";
        return -1;
    }

    if (!std::filesystem::is_regular_file(path)) {
        std::cerr << "file send failed: path is not a regular file\n";
        return -2;
    }

    std::ifstream ffile(path, std::ios::binary);
    if (!ffile) {
        std::cerr << "file send failed: could not open file\n";
        return -3;
    }

    auto const words = read_host_and_port();
    if (words.size() < 2) {
        std::cerr << "missing host or port\n";
        return -4;
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

        file.name = path.filename().string();
        file.name_len = Compatability::host_to_big_u64(
            static_cast<uint64_t>(file.name.size())
        );
        file.file_size = Compatability::host_to_big_u64(std::filesystem::file_size(path));

        asio::write(stream, asio::buffer(&file.name_len, sizeof(file.name_len)));
        asio::write(stream, asio::buffer(file.name.data(), file.name.size()));
        asio::write(stream, asio::buffer(&file.file_size, sizeof(file.file_size)));

        while (ffile.read(file.buffer.data(), static_cast<std::streamsize>(file.buffer.size()))
               || ffile.gcount() > 0) {
            std::size_t const n = static_cast<std::size_t>(ffile.gcount());
            asio::write(stream, asio::buffer(file.buffer.data(), n));
        }

        std::cout << "sent " << path << " (" << std::filesystem::file_size(path) << " bytes)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "file send failed: " << e.what() << '\n';
        return -5;
    }
}

} // namespace Whisper
