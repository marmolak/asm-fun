loader: loader.c castle.c elf-gen.c
	gcc -pipe -Wall -Wextra -g -masm=intel -o loader loader.c castle.c elf-gen.c

shellcode: shellcode.asm
	nasm -f elf64 -o shellcode.o shellcode.asm
	ld -o shellcode shellcode.o

shellcode-test: shellcode.asm
	nasm -o shellcode-test.bin shellcode.asm

exec: exec.asm
	nasm -f elf64 -o exec.o exec.asm
	ld -o exec exec.o

loader-seccomp: loader.c castle.c elf-gen.c
	gcc -pipe -Wall -Wextra -g -masm=intel -o loader loader.c castle.c elf-gen.c -lseccomp


debug: loader.c castle.c
	gcc -pipe -D DEBUG -Wall -Wextra -g -masm=intel -o loader loader.c castle.c -lseccomp
