ifdef L2_BLOCK_SIZE
CFLAGS:=-DL2_BLOCK_SIZE=$(L2_BLOCK_SIZE) $(CFLAGS)
endif

ifdef L1_BLOCK_SIZE
CFLAGS:=-DL1_BLOCK_SIZE=$(L1_BLOCK_SIZE) $(CFLAGS)
endif

CC=gcc
CFLAGS:=-Werror -O4 -funroll-loops -ffast-math -g $(CFLAGS) -march=x86-64
LDFLAGS:=-static

all: multiply

multiply: multiply.o

clean:
	rm -f multiply *.o
