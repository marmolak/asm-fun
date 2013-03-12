[bits 64]

global _start
section .text
_start:
	jmp message
dowork:
	; address of message
	pop rsi

	; syscall number - 1 write
	xor rax,rax
	inc al

	; param 1 -  stdout
	xor rdi,rdi
	inc di

	; message len
	xor rdx,rdx
	add dx, 5
	syscall

	; call exit syscall
	mov rax, 60
	xor rdi, rdi
	inc di
	inc di
	syscall
message:
	call dowork
	db 'Hell', 0AH

