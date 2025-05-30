# Compiler
CXX := x86_64-w64-mingw32-g++
CXXFLAGS := -static-libstdc++ -static-libgcc -lcrypt32 -lwininet -fvisibility=hidden -fomit-frame-pointer -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -s -Wl,--strip-all -fno-exceptions -fno-rtti

# Source and Output
SRC := src/shelly.cpp
OUT := shelly.exe
SERVER := src/shelly-server.py
PYTHON := $(shell which python3)
CWD := $(shell pwd)

.PHONY: all build

all:
	@echo "[\033[96m*\033[0m] Change the IP address in the source file, then run '\033[92mmake\033[0m build'"

build:
	@command -v $(CXX) >/dev/null 2>&1 || { echo >&2 "$(CXX) not found. Please install MinGW cross-compiler."; exit 1; }
	@echo "[\033[92m+\033[0m] Building '\033[92m$(OUT)\033[0m' ..."
	@mkdir bin
	@$(CXX) $(SRC) -o bin/$(OUT) $(CXXFLAGS)
	@echo "[\033[96m*\033[0m] File saved as: bin/\033[92m$(OUT)\033[0m"
	@echo "[\033[92m+\033[0m] Building '\033[96m$(SERVER)\033[0m ...'"
	@echo "#!/bin/bash\n\n$(PYTHON) $(CWD)/$(SERVER) \$$@" > shelly-server
	@chmod +x shelly-server
	@echo "[\033[96m*\033[0m] File saved as: \033[96mshelly-server\033[0m"
	@echo "[\033[92m+\033[0m] Build complete!!"
