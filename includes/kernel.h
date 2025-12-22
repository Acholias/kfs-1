#ifndef KERNEL_H
# define KERNEL_H

# include "types.h"

#if defined(__LINUX__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

// VGA est la grille pour le protocole video du cpu a l'adresse memoire 0xb8000
# define VGA_WIDTH		80
# define VGA_HEIGHT		25
# define VGA_MEMORY		0xB8000

# define PROMPT_LENGTH	9

# define CTRL_PRESS		0x1D
# define CTRL_RELEASE	0x9D
# define KEY_C			0x2E
# define KEY_L			0x26
# define SHIFT_LEFT		0x2A
# define SHIFT_RIGHT	0x36
# define SHIFT_LEFT_R	0xAA
# define SHIFT_RIGHT_R	0xB6
# define CAPS_LOCK		0x3A
# define ALT_PRESS		0x38
# define ALT_RELEASE	0xB8
# define LEFT_ARROW		0x4B
# define RIGHT_ARROW	0x4D
# define NUM_SCREENS	2
# define NEWLINE		'\n'
# define BACKSPACE		'\b'
# define ENTER			0x1C

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

typedef struct	s_screen
{
	size_t		save_row;
	size_t		save_column;
	u8			save_color;
	u16			save_buffer[VGA_WIDTH * VGA_HEIGHT];
}	__attribute__((packed)) t_screen;

extern	size_t		ft_strlen(const char *str);
extern	void		*ft_memcpy(void *dest, const void *src, size_t n);
extern	void		ft_memset(void *s, int c, size_t n);	

//printk
int		printk(const char *str, ...);

void	terminal_initialize();
void	terminal_set_color(u8 color);
void	set_cursor(u16 row, u16 col);
void	terminal_putentry(char c, u8 color, size_t x, size_t y);
void	terminal_scroll();
void	terminal_putchar(char c);
void	terminal_write(const char *data, size_t size);
void	clear_line();
void	handle_ctrl_c();
void	handle_backspace();
void	handle_ctrl_l();
void	handle_regular_char(char c);
void	process_scancode(u8 scancode);
void	handle_switch_terminal(u8 scancode);
void	arrow_handler(u8 scancode);
void	keyboard_handler_loop();
void	terminal_write_string(const char *data);
void	print_prompt();
void	save_screen(size_t screen_id); 
void	load_screen(size_t screen_id);
void	switch_screen(size_t new_screen_id);
void	draw_screen_index();

#endif
