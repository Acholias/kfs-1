#ifndef KERNEL_H
# define KERNEL_H

#if defined(__LINUX__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

# define VGA_WIDTH   80
# define VGA_HEIGHT  25
# define VGA_MEMORY  0xB8000 
# define PROMPT_LENGTH 9
# define CTRL_PRESS   0x1D
# define CTRL_RELEASE 0x9D

# include "types.h"

enum vga_color
{
	VGA_COLOR_BLACK,
	VGA_COLOR_BLUE,
	VGA_COLOR_GREEN,
	VGA_COLOR_CYAN,
	VGA_COLOR_RED,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_BROWN,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_DARK_GREY,
	VGA_COLOR_LIGHT_BLUE,
	VGA_COLOR_LIGHT_GREEN,
	VGA_COLOR_LIGHT_CYAN,
	VGA_COLOR_LIGHT_RED2,
	VGA_COLOR_LIGHT_MAGENTA,
	VGA_COLOR_LIGHT_BROWN,
	VGA_COLOR_WHITE,
};

extern	size_t		ft_strlen(const char *str);
extern	void		*ft_memcpy(void *dest, const void *src, size_t n);

void	print_prompt();
void	terminal_write_string(const char *data);

#endif
