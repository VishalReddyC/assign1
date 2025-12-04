# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -std=c99

# Object files
OBJS = storage_mgr.o dberror.o

# Main target
all: test_assign1

# Build test executable
test_assign1: test_assign1_1.o $(OBJS)
	$(CC) $(CFLAGS) -o test_assign1 test_assign1_1.o $(OBJS)

# Compile source files
storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	$(CC) $(CFLAGS) -c storage_mgr.c

dberror.o: dberror.c dberror.h
	$(CC) $(CFLAGS) -c dberror.c

test_assign1_1.o: test_assign1_1.c storage_mgr.h test_helper.h
	$(CC) $(CFLAGS) -c test_assign1_1.c

# Clean up
clean:
	rm -f *.o test_assign1 *.bin

# Run test
run: test_assign1
	./test_assign1

.PHONY: all clean run