bits 64

section .text
global _start

_start:
	; xores password
	mov rax, 0x6753795f5a4f5879
	push rax
	mov rax, 0x4f49584f5e7a5d4e
	push rax
	movups xmm0, [rsp]
	
	; key 42
	mov rax, 0x2a2a2a2a2a2a2a2a
	push rax
	mov rax, 0x2a2a2a2a2a2a2a2a
	push rax
	movups xmm1, [rsp]

	xorps xmm1, xmm0
