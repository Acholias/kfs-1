#ifndef IO_H
# define IO_H

#include "types.h"

__inline__
void	outb(u16 port, u8 val)
{
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}


__inline__
u8	inb(u16 port)
{
	u8	ret;

    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");	

	return (ret);
}

#endif
