CC=gcc -o3
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra
LDFLAGS=-lcrypto

.PHONY: all
all: nyufile

nyuenc: nyufile.o

nyuenc.o: nyufile.c


.PHONY: clean
clean:
	rm -f *.o nyufile
