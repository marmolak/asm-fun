shellcode: shellcode.asm
	nasm -f elf64 -o shellcode.o shellcode.asm
	ld -o shellcode shellcode.o
shellcode-test: shellcode.asm
	nasm -o shellcode-test.bin shellcode.asm

exec: exec.asm
	nasm -f elf64 -o exec.o exec.asm
	ld -o exec exec.o

loader: loader.c castle.c
	gcc -Wall -Wextra -g -masm=intel -o loader loader.c castle.c

debug: loader.c castle.c
	gcc -D DEBUG -Wall -Wextra -g -masm=intel -o loader loader.c castle.c
