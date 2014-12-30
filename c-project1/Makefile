CFILES = $(wildcard *.c)

cp1-%: cp1-%.c
	gcc -std=c99 -O0 -g -Wall -pedantic -Werror -o $@ $< -lm

all: $(basename $(CFILES))

test: all
	./test $(basename $(CFILES))

clean:
	rm $(basename $(CFILES))
