; ft_memset.s
; void	ft_memset(void *s, int c, size_t n)

section .text
	global	ft_memset

ft_memset:
	push	ebp
	mov		ebp, esp
	push	edi
	mov		edi, [ebp + 8]
	mov		eax, [ebp + 12]
	mov		ecx, [ebp + 16]

	mov		edx, edi
	cmp		ecx, 0
	je		.end

.loop:
	mov		[edi], al
	inc		edi
	dec		ecx
	jnz		.loop

.end:
	mov		eax, edx
	pop		edi
	pop		ebp
	ret
