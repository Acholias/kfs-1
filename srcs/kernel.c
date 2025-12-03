#include "../includes/stdbool.h"
#include "../includes/types.h"
#include "../includes/io.h"

#if defined(__LINUX__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

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

static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return (fg | bg << 4);
}

static inline u16	vga_entry(unsigned char uc, u8 color)
{
	return ((u16)uc | (u16)color << 8);
}

size_t	strlen(const char *str)
{
	size_t	index;

	index = 0;
	while (str[index])
		index++;
	return (index);
}

size_t					terminal_row;
size_t					terminal_column;
u8						terminal_color;
volatile u16			*terminal_buffer = (u16*)VGA_MEMORY;

void	terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_RED2, VGA_COLOR_BLACK);

	size_t	y = 0;
	while (y < VGA_HEIGHT)
	{
		size_t x = 0;
		while (x < VGA_WIDTH)
		{
			const size_t index = (y * VGA_WIDTH) + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
			x++;
		}
		y++;
	}
}

void	terminal_set_color(u8 color)
{
	terminal_color = color;
}

void	set_cursor(u16 row, u16 col)
{
	u16	pos = row * 80 + col;

	outb(0x3D4, 0x0F);
	outb(0x3D5, pos & 0xFF);

	outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

void	terminal_putentry(char c, u8 color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] =vga_entry(c, color);
}

void	terminal_putchar(char c)
{
	if (c == '\n')
	{
		terminal_row++;
		terminal_column = 0;
	}
	else
	{
		terminal_putentry(c, terminal_color, terminal_column, terminal_row);
		set_cursor(terminal_row, terminal_column + 1);
		if (++terminal_column == VGA_WIDTH)
		{
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
}

void terminal_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void	terminal_write_string(const char *data)
{
	terminal_write(data, strlen(data));
}

void	kernel_main(void)
{
	terminal_initialize();
	set_cursor(0, 0);
	terminal_write_string("Je suis pas sur de comprendre comment je catch les touches pressees !!\n");
}
