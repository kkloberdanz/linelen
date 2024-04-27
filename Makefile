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
CFLAGS=$(WARNING) $(STD) $(OPT) $(FLAGS) $(ANALYZER)

.PHONY: all
all: linelen

.PHONY: release
release: all

.PHONY: debug
debug:
	$(MAKE) OPT='-O0 -ggdb3 -Werror' all

.PHONY: asan
asan:
	$(MAKE) all OPT='-O0 -ggdb3 -fsanitize=address,leak'

.PHONY: tsan
tsan:
	$(MAKE) all OPT='-O0 -ggdb3 -fsanitize=thread'

.PHONY: ubsan
ubsan:
	$(MAKE) all OPT='-O0 -ggdb3 -fsanitize=undefined'

.PHONY: static-analysis
static-analysis:
	$(MAKE) all ANALYZER='-fanalyzer'

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
