CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c11 -pthread -Iinclude -D_GNU_SOURCE -g
LDFLAGS ?= -pthread

TARGET = architect-os

SRCS = src/main.c \
       src/parser.c \
       src/executor.c \
       src/jobs.c \
       src/memory_vmm.c \
       src/vfs_inspect.c \
       src/concurrency.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean test valgrind help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Architect OS successfully compiled: $(TARGET)"

%.o: %.c include/architect.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f /tmp/architect_os.sock /tmp/test_fifo /tmp/mmap_test.txt /tmp/test_out.txt /tmp/test_in.txt
	@echo "Clean completed."

test: $(TARGET)
	@echo "======================================================================"
	@echo "   ARCHITECT OS: COMPREHENSIVE AUTOMATED TEST SUITE (CO-1 -> CO-6)"
	@echo "======================================================================"
	@chmod +x tests/run_tests.sh 2>/dev/null || true
	@bash tests/run_tests.sh

valgrind: $(TARGET)
	@echo "======================================================================"
	@echo "   ARCHITECT OS: VALGRIND LEAK CHECK (Targeting 100% Zero Leaks)"
	@echo "======================================================================"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 \
		./$(TARGET) -c "visualize off; arch-alloc 256; heapinfo; scheduler-sim RR 2; pc-demo 6 3; exit"

help:
	@echo "Architect OS Build System Targets:"
	@echo "  make          - Build binary $(TARGET)"
	@echo "  make clean    - Remove build artifacts and temporary files"
	@echo "  make test     - Run full automated test suite for CO-1 through CO-6"
	@echo "  make valgrind - Run Valgrind memory leak verification"
