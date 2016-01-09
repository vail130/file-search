CFLAGS=-Wall -g \
	   -I $(PWD)/lib/argp/include/ \
	   -L $(PWD)/lib/argp/lib/ \
	   -l argp

all: build

clean:
	rm -rf build/*

build:
	make clean
	cc $(CFLAGS) src/filesearch.c -o build/filesearch

.PHONY: build

