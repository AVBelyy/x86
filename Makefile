CC=gcc
CFLAGS=-O2
TARGET=x86
SRC=$(TARGET).c
OUT=$(TARGET).run

all: linux
	./$(OUT)

linux: lib test
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	strip ./$(OUT)

win32: lib test
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	strip $(OUT)

lib:
	./compiler.py libc
test:
	./compiler.py test

clean:
	rm $(OUT) *.bin *.obj
start:
	geany compiler.py $(SRC) libc.asm test.asm