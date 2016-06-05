CC=gcc
LDFLAGS=-lm -std=c99 -Wall
GCC_OPT=-O2 -ftree-vectorize
GCC_OPT_HEAVY=-O3 -ftree-vectorize

CC_2=icc
ICC_OPT=-O2 
ICC_OPT_HEAVY=-O3 

MPI=mpicc
DEPS=io_funct.c

EXECUTABLES=main_prof main_safe main_gcc main_icc main_heavy main_heavy_icc main_mpi old_mpi

all: $(EXECUTABLES)

main_prof: profile_main.c
	$(CC) $< $(LDFLAGS) -pg -o $@
main_safe: main.c
	$(CC) $< $(DEPS) $(LDFLAGS) -o $@
main_gcc: main.c
	$(CC) $(GCC_OPT) $< $(DEPS) $(LDFLAGS) -o $@
main_heavy: main.c
	$(CC) $(GCC_OPT_HEAVY) $< $(DEPS) $(LDFLAGS) -o $@
main_icc: main.c
	$(CC_2) $(ICC_OPT) $< $(DEPS) $(LDFLAGS) -o $@
main_heavy_icc: main.c 
        $(CC_2) $(ICC_OPT_HEAVY) $< $(DEPS) $(LDFLAGS) -o $@
main_mpi: mpimain.c
	$(MPI) $< $(DEPS) $(LDFLAGS) -o $@
old_mpi: oldmpi.c
	$(MPI) $< $(DEPS) $(LDFLAGS) -o $@
clean:
	rm -f $(EXECUTABLES)
