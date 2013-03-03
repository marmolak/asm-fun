global _start
section text
_start:
	call where
where:
	; address of message
	pop rcx
	add cx, 38

	; syscall number
	xor rax,rax
	add al,4
	; param 1 -  stdout
	xor rbx,rbx
	inc bl
	; message len
	xor rdx,rdx
	add dl,5
	int 80h	

	; call exit syscall
	xor rax, rax
	inc al
	xor rbx,rbx
	inc bl
	inc bl
	int 80h
	db 'Hell', 0AH
