#pragma once

#include <asio.hpp>
#include <bit>
#include <boost/asio.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>
#include <asio/ssl.hpp>
// strcy for sending clipboard
struct Message {
    std::string clipboard_content;
};

// strcut for sending and receinving? files
struct File {
    std::string name;
    uint64_t name_len;
    uint64_t file_size;
    std::array<char, 8192> buffer;
};

extern File file;
extern Message message;

namespace Listen {

std::vector<std::string> where_to_hear();
int file_receiver(const std::filesystem::path& output_dir);
Message hear(asio::ip::port_type listen_port);
[[nodiscard]] bool clipsend(const std::string& text);

} // namespace Listen

namespace Whisper {

std::vector<std::string> where_to_whisper();
[[nodiscard]] bool send(std::vector<std::string>& words);
void clipread();
int file_sender(std::filesystem::path& path);
int file_sender(const std::filesystem::path& path);

} // namespace Whisper

namespace Compatability {

constexpr uint64_t host_to_big_u64(uint64_t x) {
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

// template for logging
template <typename T>
concept writable = requires(T x) { std::cout << x; };

namespace DEBUG {

template <writable T>
void log(const T& text);

} // namespace DEBUG

const char* read_clipboard();
bool write_clipboard(const std::string&);
