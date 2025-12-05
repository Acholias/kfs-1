; ft_strlen
; size_t	ft_strlen(const char *s)


global	ft_strlen

ft_strlen:
	push	ebp
	mov		ebp, esp
	mov		eax, 0
	mov		edi, [ebp + 8]

.loop:
	cmp		byte [edi + eax], 0
	je		.end
	inc		eax
	jmp		.loop

.end
	pop	ebp
	ret
