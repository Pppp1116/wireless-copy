#include "header.hpp"


#include <cstdlib>

const char* read_clipboard() {
#ifdef __linux__
    if (std::getenv("WAYLAND_DISPLAY")) {
        return "wl-paste -n";
    }
    if (std::getenv("DISPLAY")) {
        return "xclip -selection clipboard -o";
    }
    return nullptr;

#elif _WIN32
    return "powershell -command Get-Clipboard";

#elif __APPLE__
    return "pbpaste";

#else
    return nullptr;
#endif
}
