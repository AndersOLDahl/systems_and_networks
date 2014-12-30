CFILES = $(wildcard *.c)

cp%: cp%.c
	gcc -g -std=c99 -O0 -o $@ $< -lm

all: $(basename $(CFILES))

test: all
	./test $(basename $(CFILES))

clean:
	rm $(basename $(CFILES))
