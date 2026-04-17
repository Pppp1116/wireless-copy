#include "header.hpp"

[[nodiscard]] bool send(std::vector<std::string>& words) {
    if (words.size() < 2) {
        std::cerr << "missing port\n";
        return false;
    }

    int parsed_port;
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

    asio::ip::tcp::endpoint endpoint(
        asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(parsed_port)
    );

    try {
        asio::io_context io;
        asio::ip::tcp::acceptor acceptor(io, endpoint);
        asio::ip::tcp::socket socket(io);

        acceptor.accept(socket);

        std::array<char, 1024> buffer{};
        std::size_t n = socket.read_some(asio::buffer(buffer));

        std::cout.write(buffer.data(), static_cast<std::streamsize>(n));
        std::cout << '\n';

        asio::write(socket, asio::buffer(&message, sizeof(message)));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "network error: " << e.what() << '\n';
        return false;
    }
} //TODO add calling and need to read clip write ti strucr and write to remote clip