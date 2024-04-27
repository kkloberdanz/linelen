CC=cc
STD=-std=c89
OPT=-Os -flto
LDFLAGS=
WARNING=-Wall -Wextra -Wpedantic -Wfloat-equal -Wundef -Wshadow \
	-Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
	-Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual \
	-Wswitch-enum -Wunreachable-code -Wformat -Wformat-security -Wvla \
	-Werror=implicit-function-declaration -Wno-error=cpp

FLAGS=-g -fstack-protector-all -D_FORTIFY_SOURCE=2 -fpie -pipe
CFLAGS=$(WARNING) $(STD) $(OPT) $(FLAGS)

.PHONY: all
all: linelen

.PHONY: release
release: all

.PHONY: debug
debug:
	$(MAKE) OPT='-O0 -ggdb3' all

linelen: main.c
	$(CC) -o linelen main.c $(CFLAGS) $(LDFLAGS)

.PHONY: fmt
fmt:
	clang-format -i *.c

.PHONY: clean
clean:
	rm -f *.o
	rm -f linelen
	rm -rf linelen.dSYM
