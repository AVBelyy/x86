all: linux
linux:
	./compiler.py libc
	./compiler.py test
	gcc -O2 x86.c lib/linux.c -o x86 && ./x86
win32:
	./compiler.py libc
	./compiler.py test
	gcc x86.c lib/win32.c -o x86
	x86.exe
clean:
	rm x86 *.bin *.obj
start:
	geany compiler.py x86.c libc.asm test.asm &
gtk:
	@gcc `pkg-config --libs gtk+-2.0` `pkg-config --cflags gtk+-2.0` hw.c -o hw
	@./hw
