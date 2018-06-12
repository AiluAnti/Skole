# Makefile for compiling and linking .C files with asm files

# -m32 compiles .C to 32bit assembly
# -masm=att Compiles .C with at&t assembly systax

CC = gcc
CFLAGS = -m32 -masm=att -g

NUM_ELEMS = 9
NUM_RUNS = 100000

all: program

# To change optimization levels on .C file change -O0 to either -O1, -O2, -O3
# Optimization levels GCC https://gcc.gnu.org/onlinedocs/gnat_ugn/Optimization-Levels.html
main: main.c Makefile
	$(CC) $(CFLAGS) -O0 -S -o $@.s $<  

# asm.S and main has to exist before executing compilation targets
program: benchmark.S main
	$(CC) $(CFLAGS) -E $< > $@_pp.s
	$(CC) $(CFLAGS) main.s $@_pp.s -o $@

test:
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
	./program $(NUM_ELEMS) $(NUM_RUNS)
