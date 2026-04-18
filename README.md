# wireless-copy

`wireless-copy` is a small C++ tool for sending clipboard contents and files between machines over a TLS-protected TCP connection.

Right now the project supports:

- Listening for remote clipboard text and writing it into the local clipboard
- Reading the local clipboard and sending it to another machine
- Receiving a file into a chosen output directory
- Sending a file to another machine
- Interactive mode when no arguments are provided

Current limitations:

- The CMake project currently builds an executable named `rdbg`
- The listener expects TLS certificate files at `certs/server.crt` and `certs/server.key`
- File mode is currently interactive only through menu options `3` and `4`

## Requirements

- CMake 3.20 or newer
- A C++26-capable compiler
- OpenSSL
- Threads
- Asio

Clipboard tools depend on your platform:

- Linux Wayland: `wl-copy` and `wl-paste`
- Linux X11: `xclip`
- macOS: `pbpaste` for reading; clipboard write support is not implemented here
- Windows: PowerShell `Get-Clipboard` for reading; clipboard write support is not implemented here

## Build

```bash
cmake -S . -B build
cmake --build build
```

The binary is written to `bin/rdbg`.

## Run

Start in interactive mode:

```bash
./bin/rdbg
```

Run directly in clipboard listen mode:

```bash
./bin/rdbg listen
```

Run directly in clipboard send mode:

```bash
./bin/rdbg whisper
```

Menu-driven file modes are also available:

- `3` receives a file
- `4` sends a file

When prompted:

- `listen` asks for a port number
- `whisper` asks for `host port`
- `3` asks for an output directory first, then the listen port
- `4` asks for a file path first, then `host port`

## File Transfer

To receive a file:

```bash
./bin/rdbg
```

Then choose option `3`, enter the destination directory, and enter the port to listen on.

To send a file:

```bash
./bin/rdbg
```

Then choose option `4`, enter the path to the file, and enter `host port` for the remote receiver.

The receiver saves the incoming file under its original filename inside the selected destination directory.

## TLS Certificates

Listener modes load these files relative to the project directory:

- `certs/server.crt`
- `certs/server.key`

Create or place valid server certificates there before using listener mode.

## License

This project is licensed under the GNU General Public License v2. See [LICENSE](LICENSE).
