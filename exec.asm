[bits 64]

section .data
shell: db "/bin/cat", 0, "secret", 0

global _start
section .text
_start:
	call where
where:
	pop rsi

	; 1 - address of /bin/cat string
	push shell
	pop rdi


	; execve
	xor rax, rax
	add rax, 59
	
	; array
	push 0
	add rdi, 9
	push rdi
	sub rdi, 9
	push rdi

	; 2 - array address
	mov rsi, rsp
	; 3 - NULL
	xor rdx, rdx
	
	syscall
