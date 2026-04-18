#include "header.hpp"

namespace Whisper {

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
        std::cerr << "missing port\n";
        return false;
    }

    int parsed_port = 0;
    try {
        parsed_port = std::stoi(words[1]);
    } catch (const std::exception&) {
        std::cerr << "invalid port\n";
        return false;
    }

    if (parsed_port < 1 || parsed_port > 65535) {
        std::cerr << "invalid port\n";
        return false;
    }

    asio::ip::tcp::endpoint const endpoint(
        asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(parsed_port)
    );

    try {
        asio::io_context io;
        asio::ip::tcp::acceptor acceptor(io, endpoint);
        asio::ip::tcp::socket socket(io);

        acceptor.accept(socket);

        std::array<char, 1024> buffer{};
        std::size_t const n = socket.read_some(asio::buffer(buffer));

        std::cout.write(buffer.data(), static_cast<std::streamsize>(n));
        std::cout << '\n';

        asio::write(socket, asio::buffer(&message, sizeof(message)));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "network error: " << e.what() << '\n';
        return false;
    }
} //TODO add calling and need to read clip write ti strucr and write to remote clip

void clipread() {
    const char* cmd =
        std::getenv("WAYLAND_DISPLAY") ? "wl-paste -n" : "xclip -selection clipboard -o";

    FILE* pipe = popen(cmd, "r");
    if (pipe == nullptr) {
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        message.clipboard_content += buffer;
    }

    pclose(pipe);
}
int file_sender(const std::filesystem::path& path) {
    std::ifstream ffile(path, std::ios::binary);
    if (!ffile) {
        return -1;
    }

    file.name = path.filename().string();
    file.name_len = Compatability::host_to_big_u64(file.name.size());
    file.file_size = Compatability::host_to_big_u64(std::filesystem::file_size(path));

    asio::write(socket, asio::buffer(&file.name_len, sizeof(file.name_len)));
    asio::write(socket, asio::buffer(file.name.data(), file.name.size()));
    asio::write(socket, asio::buffer(&file.file_size, sizeof(file.file_size)));

    while (ffile.read(file.buffer.data(), file.buffer.size()) || ffile.gcount() > 0) {
        std::size_t n = static_cast<std::size_t>(ffile.gcount());
        asio::write(socket, asio::buffer(file.buffer.data(), n));
    }

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
} // namespace Comlpatability