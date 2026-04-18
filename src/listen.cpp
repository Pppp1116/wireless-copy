#include "header.hpp"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace Listen {

namespace {
asio::ip::port_type parse_port(const std::vector<std::string>& words) {
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

std::filesystem::path sanitize_received_filename(const std::string& raw_name) {
    std::filesystem::path const candidate(raw_name);
    std::filesystem::path const filename = candidate.filename();

    if (filename.empty() || filename != candidate) {
        throw std::runtime_error("received file name is invalid");
    }

    return filename;
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
    try {
        auto const words = read_words("port number ");
        auto const listen_port = parse_port(words);

        asio::io_context io;
        asio::ssl::context ctx(asio::ssl::context::tls_server);

        ctx.use_certificate_chain_file("certs/server.crt");
        ctx.use_private_key_file("certs/server.key", asio::ssl::context::pem);

        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), listen_port);
        asio::ip::tcp::acceptor acceptor(io, endpoint);
        asio::ssl::stream<asio::ip::tcp::socket> stream(io, ctx);

        std::cout << "waiting for file on port "
                  << acceptor.local_endpoint().port()
                  << '\n';

        acceptor.accept(stream.next_layer());
        stream.handshake(asio::ssl::stream_base::server);

        uint64_t be_name_len = 0;
        uint64_t be_file_size = 0;

        asio::read(stream, asio::buffer(&be_name_len, sizeof(be_name_len)));
        std::uint64_t const name_len = Compatability::big_to_host(be_name_len);

        if (name_len == 0 || name_len > 4096) {
            throw std::runtime_error("received file name length is invalid");
        }

        std::string raw_name(name_len, '\0');
        asio::read(stream, asio::buffer(raw_name.data(), raw_name.size()));

        asio::read(stream, asio::buffer(&be_file_size, sizeof(be_file_size)));
        std::uint64_t const file_size = Compatability::big_to_host(be_file_size);

        std::filesystem::create_directories(output_dir);

        std::filesystem::path const safe_name = sanitize_received_filename(raw_name);
        std::filesystem::path const out_path = output_dir / safe_name;

        std::ofstream ofile(out_path, std::ios::binary);
        if (!ofile) {
            std::cerr << "failed to open output file: " << out_path << '\n';
            return -1;
        }

        std::array<char, 8192> buffer{};
        std::uint64_t remaining = file_size;

        while (remaining > 0) {
            std::size_t const chunk = static_cast<std::size_t>(
                std::min<std::uint64_t>(buffer.size(), remaining)
            );

            asio::read(stream, asio::buffer(buffer.data(), chunk));
            ofile.write(buffer.data(), static_cast<std::streamsize>(chunk));

            if (!ofile) {
                std::cerr << "failed while writing output file\n";
                return -2;
            }

            remaining -= static_cast<std::uint64_t>(chunk);
        }

        std::cout << "received " << out_path << " (" << file_size << " bytes)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "file receive failed: " << e.what() << '\n';
        return -3;
    }
}

} // namespace Listen
