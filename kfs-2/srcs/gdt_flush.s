; **************************************************************************** ;
;                                                                              ;
;                                                         :::      ::::::::    ;
;    gdt_flush.s                                        :+:      :+:    :+:    ;
;                                                     +:+ +:+         +:+      ;
;    By: lumugot <lumugot@42angouleme.fr>           +#+  +:+       +#+         ;
;                                                 +#+#+#+#+#+   +#+            ;
;    Created: 2026/01/20 11:46:02 by lumugot           #+#    #+#              ;
;    Updated: 2026/01/20 11:50:59 by lumugot          ###   ########.fr        ;
;                                                                              ;
; **************************************************************************** ;

BITS	32

global	gdt_flush

gdt_flush:
	mov		eax, [esp + 4]
	lgdt	[eax]

	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	jmp		0x08:.flush

.flush:
	ret
