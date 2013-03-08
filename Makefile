shellcode: shellcode.asm
	nasm -f elf64 -o shellcode.o shellcode.asm
	ld -o shellcode shellcode.o

loader: loader.c
	gcc -Wall -Wextra -g -masm=intel -o loader loader.c

debug: loader.c
	gcc -D DEBUG -Wall -Wextra -g -masm=intel -o loader loader.c
