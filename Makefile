CC=gcc
LIBS=
CFLAGS=-O2 -w
TARGET=x86
SRC=src/$(TARGET).c src/lib/core/*.c
OUT=$(TARGET)
APPS=lib/libc.asm examples/*.asm

AVR_MCU=at90usb1286
AVR_CFLAGS=$(CFLAGS) -mmcu=$(AVR_MCU) -D_AVR -D__AVR_`echo $(AVR_MCU) | tr 'a-z' 'A-Z'`__ -DF_CPU=16000000 -DUSB_SERIAL \
                     -Isrc/lib/platform/$(AVR_MCU) -include src/lib/platform/$(AVR_MCU)/hal.h
AVR_LIBS=

all: linux

linux: apps
	$(CC) $(CFLAGS) $(SRC) src/lib/platform/linux.c $(LIBS) -o $(OUT)

win32: apps
	$(CC) $(CFLAGS) $(SRC) src/lib/platform/win32.c $(LIBS) -o $(OUT).exe

avr: apps
	avr-gcc $(AVR_CFLAGS) -c src/lib/platform/$(AVR_MCU)/*.c
	avr-gcc $(AVR_CFLAGS) $(SRC) src/lib/platform/avr.c $(AVR_LIBS) -o $(OUT)
	avr-objcopy -O ihex $(OUT) $(OUT).hex
	rm *.o

apps:
	@RESULT=$(foreach APP, $(APPS), $(shell sh -cx "./compiler.py $(APP)"))

clean:
	rm -f $(OUT) $(OUT).exe *.bin *.obj *.o
