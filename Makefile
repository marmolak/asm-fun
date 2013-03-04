shellcode:
	nasm -f elf64 -o shellcode.o shellcode.asm
	ld -o shellcode shellcode.o

loader:
	gcc -g -masm=intel -o loader loader.c
