shellcode: shellcode.asm
	nasm -f elf64 -o shellcode.o shellcode.asm
	ld -o shellcode shellcode.o

exec: exec.asm
	nasm -f elf64 -o exec.o exec.asm
	ld -o exec exec.o

loader: loader.c
	gcc -Wall -Wextra -g -masm=intel -o loader loader.c

debug: loader.c
	gcc -D DEBUG -Wall -Wextra -g -masm=intel -o loader loader.c
