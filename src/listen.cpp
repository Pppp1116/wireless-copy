#include "header.hpp"

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
Message hear(const std::vector<std::string>& words) {
    if (words.empty()) {
        std::cerr << "missing port\n";
        std::exit(1);
    }

    try {
        int const port = std::stoi(words[0]);

        asio::io_context io;

        asio::ip::tcp::endpoint const endpoint(asio::ip::tcp::v4(), (asio::ip::port_type)port);
        asio::ip::tcp::acceptor acceptor(io, endpoint);

        asio::ip::tcp::socket socket(io);

        acceptor.accept(socket); // wait for connection

        asio::read(socket, asio::buffer(&message, sizeof(message)));

        // now message is fully received
        return message;
    } catch (const std::exception& e) {
        std::cerr << "invalid port\n";
        std::exit(1);
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
    uint64_t be_name_len = 0;
    uint64_t be_file_size = 0;

    // Read metadata
    asio::read(socket, asio::buffer(&be_name_len, sizeof(be_name_len)));
    std::uint64_t name_len = Compatability::big_to_host(be_name_len);

    std::string name(name_len, '\0');
    asio::read(socket, asio::buffer(name.data(), name.size()));

    asio::read(socket, asio::buffer(&be_file_size, sizeof(be_file_size)));
    std::uint64_t file_size = Compatability::big_to_host(be_file_size);

    // Open output file
    std::filesystem::path out_path = output_dir / name;
    std::ofstream ofile(out_path, std::ios::binary);
    if (!ofile) {
        return -1;
    }

    // Read file contents
    std::array<char, 4096> buffer;
    std::uint64_t remaining = file_size;

    while (remaining > 0) {
        std::size_t chunk = static_cast<std::size_t>(
            std::min<std::uint64_t>(buffer.size(), remaining)
        );

        asio::read(socket, asio::buffer(buffer.data(), chunk));
        ofile.write(buffer.data(), chunk);

        if (!ofile) {
            return -2;
        }

        remaining -= chunk;
    }

    return 0;
}

} // namespace Listen


