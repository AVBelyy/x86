CC=gcc
CFLAGS=-O2
TARGET=x86
SRC=$(TARGET).c
OUT=$(TARGET)
APPS=libc.asm examples/*.asm

all: linux

linux: apps
	$(CC) $(CFLAGS) $(SRC) lib/linux.c -o $(OUT)
	./$(OUT)

win32: apps
	$(CC) $(CFLAGS) $(SRC) lib/win32.c -o $(OUT).exe
	$(OUT).exe

apps:
	@RESULT=$(foreach APP, $(APPS), $(shell sh -cx "./compiler.py $(APP)"))

clean:
	rm -f $(OUT) $(OUT).exe *.bin *.obj
