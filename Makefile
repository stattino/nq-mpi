CC_1=gcc
CC_2=icc
CFLAGS=-g
LDFLAGS= -lm -std=c99

GCC_OPT=-g -03 -ftree-vectorize

MKL_FLAGS=-DMKLINUSE -DMKL_ILP64 -I${MKLROOT}/include -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_sequential -lpthread
ICC_OPT=-O3 -xHost

EXECUTABLES=main

all: $(EXECUTABLES)

main_safe: main.c
	$(CC_1) $< $(CFLAGS) $(LDFLAGS)
main_gcc: main.c
	$(CC_1) $< $(GCC_OPT) -o $@ $(LDFLAGS)
main_icc: main.c
	$(CC_2) $< $(ICC_OPT) -o  $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES)
