# Makefile for SonOptimizasyon (Optimization)
# Usage:
#   make          -> builds the main example (src/main)
#   make solver   -> builds the benchmark solver (src/benchmarking/solver)
#   make clean    -> remove build artifacts

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I/opt/homebrew/include
LDFLAGS := -L/opt/homebrew/lib -lboost_filesystem -lboost_system

SRCDIR := .
BENCHDIR := benchmarking

MAIN_SRC := $(SRCDIR)/main.cpp
MAIN_BIN := $(SRCDIR)/main
BENCH_SRC := $(BENCHDIR)/benchmark.cpp
BENCH_BIN := $(BENCHDIR)/solver

.PHONY: all solver clean help

all: $(MAIN_BIN)
	@echo "Built: $(MAIN_BIN)"

$(MAIN_BIN): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(MAIN_BIN)

solver: $(BENCH_BIN)
	@if [ -f $(BENCH_BIN) ]; then \
		echo "Built: $(BENCH_BIN)"; \
	else \
		echo "Warning: $(BENCH_BIN) not built (Boost missing)"; \
	fi

$(BENCH_BIN): $(BENCH_SRC)
	@echo "Checking for Boost.Filesystem..."
	@echo '#include <boost/filesystem.hpp>' > /tmp/boost_check.cpp
	@echo 'int main() { return 0; }' >> /tmp/boost_check.cpp
	@if g++ -std=c++17 /tmp/boost_check.cpp -o /tmp/boost_check 2>/dev/null; then \
		$(CXX) $(CXXFLAGS) $(BENCH_SRC) -o $(BENCH_BIN) $(LDFLAGS); \
		echo "Built: $(BENCH_BIN)"; \
	else \
		echo "Warning: Boost.Filesystem not found. Skipping $(BENCH_BIN) build."; \
		rm -f /tmp/boost_check.cpp; \
	fi
	@rm -f /tmp/boost_check


clean:
	rm -f $(MAIN_BIN) $(BENCH_BIN)
	@echo "Cleaned build artifacts"

help:
	@echo "Makefile targets:" 
	@echo "  make        -> build main example (src/main)"
	@echo "  make solver -> build benchmarking solver (src/benchmarking/solver)"
	@echo "  make clean  -> remove built binaries"
	@echo "Notes: Do not compile header (.h) files directly; use 'make' or call g++ on .cpp translation units."
