CC=gcc
CFLAGS=-O2
TARGET=x86
SRC=$(TARGET).c
OUT=$(TARGET)
APPS=libc test examples/qsort

all: linux

linux: apps
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	./$(OUT)

win32: apps
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	$(OUT).exe

apps:
	@RESULT=$(foreach APP, $(APPS), $(shell sh -cx "./compiler.py $(APP)"))

clean:
	rm $(OUT) *.bin *.obj
start:
	geany compiler.py $(SRC) libc.asm test.asm
