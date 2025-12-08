#include "../includes/kernel.h"
#include "../includes/stdbool.h"
#include "../includes/types.h"
#include "../includes/io.h"

static const char scancode_to_ascii[128] = {
    0,27,'1','2','3','4','5','6','7','8',
    '9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p',
    '[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';',
    '\'','`',0,'\\','z','x','c','v','b','n',
    'm',',','.','/',0,'*',0,' ',0,0
};

static const char scancode_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b', '\t',
    'Q','W','E','R','T','Y','U','I','O','P',
    '{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':',
    '"','~',0,'|','Z','X','C','V','B','N',
    'M','<','>','?',0,'*',0,' '
};

static bool	shift_pressed =	false;
static bool	caps_lock =	false;
static bool	ctrl_pressed = false;

static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return (fg | bg << 4);
}

static inline u16	vga_entry(unsigned char uc, u8 color)
{
	return ((u16)uc | (u16)color << 8);
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

void	terminal_scroll()
{
	size_t	bytes_copy = (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(u16);	
	ft_memcpy((void*)terminal_buffer, (void*)(terminal_buffer + VGA_WIDTH), bytes_copy);

	size_t	x = 0;
	while (x < VGA_WIDTH)
	{
		size_t	index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
		terminal_buffer[index] = vga_entry(' ', terminal_color);
		++x;
	}
	terminal_row = VGA_HEIGHT - 1;
	terminal_column = 0;
}

void	terminal_putchar(char c)
{
	if (c == '\n')
	{
		terminal_row++;
		terminal_column = 0;

		if (terminal_row >= VGA_HEIGHT)
			terminal_scroll();
	
		print_prompt();
	}
	else
	{
		terminal_putentry(c, terminal_color, terminal_column, terminal_row);
		++terminal_column;
		if (terminal_column >= VGA_WIDTH)
		{
			terminal_column = 0;
			++terminal_row;
			if (terminal_row >= VGA_HEIGHT)
				terminal_scroll();
		}
		set_cursor(terminal_row, terminal_column + 1);
	}
}

void terminal_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void	clear_line()
{
	size_t	x = PROMPT_LENGTH;
	while (x < VGA_WIDTH)
	{
		terminal_putentry(' ', terminal_color, x, terminal_row);
		++x;
	}
	terminal_column = PROMPT_LENGTH;
	set_cursor(terminal_row, terminal_column);
	print_prompt();
}

void	handle_ctrl_c()
{
	u8	old_color = terminal_color;
	terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED2, VGA_COLOR_BLACK));
	
	terminal_putentry('^', terminal_color, terminal_column, terminal_row);
	terminal_column++;
	terminal_putentry('C', terminal_color, terminal_column, terminal_row);
	terminal_column++;
	
	terminal_set_color(old_color);
	terminal_column = 0;
	terminal_row++;
	
	if (terminal_row >= VGA_HEIGHT)
		terminal_scroll();
	
	print_prompt();
}

void Keyboard_handler_loop()
{
	while (1)
	{
		if (inb(0x64) & 1)
		{
			u8 sc = inb(0x60);

			if (sc == CTRL_PRESS)
				ctrl_pressed = true;
			else if (sc == CTRL_RELEASE)
				ctrl_pressed = false;
			else if (ctrl_pressed && sc == 0x2E)
				handle_ctrl_c();
			else if (ctrl_pressed && sc == 0x26)
			{
				terminal_initialize();
				print_prompt();
			}
			else if (sc == 0x2A || sc == 0x36)
				shift_pressed = true;
			else if (sc == 0xAA || sc == 0xB6)
				shift_pressed = false;
			else if (sc == 0x3A)
				caps_lock = !caps_lock;
			else if (sc < 128 && !ctrl_pressed)
			{
				char c;
				if (shift_pressed)
					c = scancode_shift[sc];
				else
					c = scancode_to_ascii[sc];

				if (c == '\b')
				{
					if (terminal_column > PROMPT_LENGTH)
					{
						terminal_column--;
						terminal_putentry(' ', terminal_color, terminal_column, terminal_row);
						set_cursor(terminal_row, terminal_column);
					}
				}
				else if (c)
				{
					if (caps_lock && c >= 'a' && c <= 'z')
						c -= 32;
					terminal_putchar(c);
				}
			}
		}
	}
}

void	terminal_write_string(const char *data)
{
	terminal_write(data, ft_strlen(data));
}

void	print_prompt()
{
	u8 old_color = terminal_color;
	terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	terminal_write_string("kfs-1 -> ");
	terminal_set_color(old_color);
	set_cursor(terminal_row, terminal_column);
}

void	kernel_main(void)
{
	terminal_initialize();
	print_prompt();
	Keyboard_handler_loop();
}
