CC=gcc
CFLAGS=-O2
TARGET=x86
SRC=$(TARGET).c
OUT=$(TARGET).run

all: linux
	./compiler.py libc
	./compiler.py test
	./$(OUT)

linux:
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	strip ./$(OUT)

win32:
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	strip $(OUT)

clean:
	rm $(OUT) *.bin *.obj
start:
	geany compiler.py $(SRC) libc.asm test.asm