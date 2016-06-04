CC=gcc
LDFLAGS=-lm -std=c99 -Wall
GCC_OPT=-g -O2 -ftree-vectorize
GCC_OPT_HEAVY=-O3 -ftree-vectorize

MKL_FLAGS=-DMKLINUSE -DMKL_ILP64 -I${MKLROOT}/include -L${MKLROOT}/lib/intel64 -lmkl_in
tel_ilp64 -lmkl_core -lmkl_sequential -lpthread
ICC_OPT=-O3 -xHost

MPI=mpicc


EXECUTABLES=main_safe main_gcc main_heavy main_mpi old_mpi
#main_icc
all: $(EXECUTABLES)
main_safe: main.c
	$(CC) $(C_FLAGS) $< $(LDFLAGS) -o $@
main_gcc: main.c
	$(CC) $(GCC_OPT) $< $(LDFLAGS) -o $@
main_heavy: main.c
	$(CC) $(GCC_OPT_HEAVY) $< $(LDFLAGS) -o $@
main_mpi: mpimain.c
	$(MPI) $< $(LDFLAGS) -o $@
old_mpi: oldmpi.c
	$(MPI) $< $(LDFLAGS) -o $@
#main_icc: main.c
#	icc $(ICC_OPT) $< $(MKL_FLAGS) -lm -o $@
clean:
	rm -f $(EXECUTABLES)
