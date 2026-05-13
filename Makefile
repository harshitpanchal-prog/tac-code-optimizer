# ============================================================
#  TAC Code Optimizer — Makefile
#  Targets:
#    make          → build ./optimizer  (or optimizer.exe on Windows)
#    make clean    → remove build artifacts
#    make rebuild  → clean + build
# ============================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I include

# Source files
SRCS := src/main.cpp \
        src/parser.cpp \
        src/basicblock.cpp \
        src/cfg.cpp \
        src/optimizer.cpp \
        src/utils.cpp

# Detect OS — use .exe on Windows
ifeq ($(OS), Windows_NT)
    TARGET := optimizer.exe
    RM     := del /Q
else
    TARGET := optimizer
    RM     := rm -f
endif

# ── Default target ────────────────────────────────────────
.PHONY: all
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	@echo "Build successful → $(TARGET)"

# ── Clean ─────────────────────────────────────────────────
.PHONY: clean
clean:
	$(RM) $(TARGET)
	@echo "Cleaned build artifacts."

# ── Rebuild ───────────────────────────────────────────────
.PHONY: rebuild
rebuild: clean all
