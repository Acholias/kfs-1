#include "../includes/kernel.h"
#include "../includes/stdbool.h"
#include "../includes/io.h"
#include "../includes/gdt.h"

size_t			terminal_row = 0;
size_t			terminal_column = 0;
u8				terminal_color = 0;
volatile u16	*terminal_buffer = 0;
size_t			current_screen = 0;
t_screen		screens[NUM_SCREENS];

static bool	shift_pressed =	false;
static bool	caps_lock =	false;
static bool	ctrl_pressed = false;
static bool alt_pressed = false;

static	char	input_buffer[INPUT_MAX];
static	size_t	input_len = 0;

static const char scancode_to_ascii[128] = {
    0,27,'1','2','3','4','5','6','7','8',
    '9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p',
    '[',']',0,0,
    'a','s','d','f','g','h','j','k','l',';',
    '\'','`',0,'\\','z','x','c','v','b','n',
    'm',',','.','/',0,'*',0,' ',0,0
};

static const char scancode_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b', '\t',
    'Q','W','E','R','T','Y','U','I','O','P',
    '{','}',0,0,
    'A','S','D','F','G','H','J','K','L',':',
    '"','~',0,'|','Z','X','C','V','B','N',
    'M','<','>','?',0,'*',0,' '
};


static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return (fg | bg << 4);
}

static inline u16	vga_entry(unsigned char uc, u8 color)
{
	return ((u16)uc | (u16)color << 8);
}

void	terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_RED2, VGA_COLOR_BLACK);
	terminal_buffer = (u16*)VGA_MEMORY;
	current_screen = 0;
	
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
	print_prompt();
	for (size_t s = 0; s < NUM_SCREENS; ++s)
	{
		screens[s].save_row = 0;
		screens[s].save_column = 0;
		screens[s].save_color = vga_entry_color(VGA_COLOR_LIGHT_RED2, VGA_COLOR_BLACK);	
		ft_memcpy(screens[s].save_buffer, (void *)terminal_buffer, VGA_WIDTH * VGA_HEIGHT * sizeof(u16));
	}
}

void	terminal_clear_screen()
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);

    terminal_row = 0;
    terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_RED2, VGA_COLOR_BLACK);
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
	if (c == NEWLINE)
	{
		terminal_row++;
		terminal_column = 0;

		if (terminal_row >= VGA_HEIGHT)
			terminal_scroll();
		set_cursor(terminal_row, terminal_column);
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
		set_cursor(terminal_row, terminal_column);
	}
}

void	terminal_write(const char *data, size_t size)
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

void	redraw_input_line()
{
	set_cursor(terminal_row, terminal_column);
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

void	handle_backspace()
{
	if (terminal_column > PROMPT_LENGTH)
	{
		--input_len;
		input_buffer[input_len] = 0;

		--terminal_column;
		terminal_putentry(' ', terminal_color, terminal_column, terminal_row);
		set_cursor(terminal_row, terminal_column);
	}
}

void	handle_ctrl_l()
{
	terminal_clear_screen();
	print_prompt();
}

void	handle_regular_char(char c)
{
	if (caps_lock && c >= 'a' && c <= 'z')
		c -= 32;

	if (terminal_column >= VGA_WIDTH - 1)
		return ;

	input_buffer[input_len++] = c;
	input_buffer[input_len] = 0;

	terminal_putchar(c);
}

void	handle_enter()
{
	terminal_putchar('\n');
	execute_command(input_buffer);
	input_len = 0;
	print_prompt();
}

void	process_scancode(u8 scancode)
{
	char c;
	
	if (scancode == ENTER)
	{
		handle_enter();
		return ;
	}
	
	if (shift_pressed)
		c = scancode_shift[scancode];
	else
		c = scancode_to_ascii[scancode];
	
	if (c == BACKSPACE)
		handle_backspace();
	else if (c)
		handle_regular_char(c);
}

void	handle_switch_terminal(u8 scancode)
{
	if (scancode == ALT_PRESS)
		alt_pressed = true;
	else if (scancode == ALT_RELEASE)
		alt_pressed = false;
	
	else if (alt_pressed && scancode == LEFT_ARROW)
	{
		size_t	new_screen = (current_screen == 0) ? NUM_SCREENS - 1 : current_screen - 1;
		switch_screen(new_screen);
	}
	else if (alt_pressed && scancode == RIGHT_ARROW)
	{
		size_t	new_screen = (current_screen + 1) % NUM_SCREENS;
		switch_screen(new_screen);
	}
}

void	arrow_handler(u8 scancode)
{
	if (scancode == LEFT_ARROW)
	{
		if (terminal_column > PROMPT_LENGTH)
		{
			--terminal_column;
			set_cursor(terminal_row, terminal_column);
		}
	}
	else if (scancode == RIGHT_ARROW)
	{
		if (terminal_column < VGA_WIDTH - 1)
		{
			++terminal_column;
			set_cursor(terminal_row, terminal_column);
		}
	}
}

void	keyboard_handler_loop()
{
	while (1)
	{
		if (inb(0x64) & 1)
		{
			u8 scancode = inb(0x60);
			
			handle_switch_terminal(scancode);
			if (!alt_pressed && (scancode == RIGHT_ARROW || scancode == LEFT_ARROW))
				arrow_handler(scancode);
			else if (scancode == CTRL_PRESS)
				ctrl_pressed = true;
			else if (scancode == CTRL_RELEASE)
				ctrl_pressed = false;
			else if (ctrl_pressed && scancode == KEY_C)
				handle_ctrl_c();
			else if (ctrl_pressed && scancode == KEY_L)
				handle_ctrl_l();
			else if (scancode == SHIFT_LEFT || scancode == SHIFT_RIGHT)
				shift_pressed = true;
			else if (scancode == SHIFT_LEFT_R || scancode == SHIFT_RIGHT_R)
				shift_pressed = false;
			else if (scancode == CAPS_LOCK)
				caps_lock = !caps_lock;
			else if (scancode < 128 && !ctrl_pressed)
				process_scancode(scancode);
		}
	}
}

void	terminal_write_string(const char *data)
{
	size_t i = 0;
	while (data[i])
	{
		if (data[i] == '\n')
			terminal_putchar('\n');
		else
			terminal_putchar(data[i]);
		i++;
	}
}

void	print_prompt()
{
	u8 old_color = terminal_color;
	terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	size_t i = 0;
	const char *prompt = "kfs-1 -> ";
	while (prompt[i])
	{
		terminal_putchar(prompt[i]);
		i++;
	}
	terminal_set_color(old_color);
	draw_screen_index();
	set_cursor(terminal_row, PROMPT_LENGTH);
}

void	save_screen(size_t screen_id) 
{
	if (screen_id >= NUM_SCREENS)
		return ;

	ft_memcpy(screens[screen_id].save_buffer, (void*)terminal_buffer,
		   VGA_WIDTH * VGA_HEIGHT * sizeof(u16));

	screens[screen_id].save_row = terminal_row;
	screens[screen_id].save_column = terminal_column;
	screens[screen_id].save_color = terminal_color;
}

void	load_screen(size_t screen_id)
{
	if (screen_id >= NUM_SCREENS)
		return ;

	ft_memcpy((void*)terminal_buffer, screens[screen_id].save_buffer,
		   VGA_WIDTH * VGA_HEIGHT * sizeof(u16));

	terminal_row = screens[screen_id].save_row;
	terminal_column = screens[screen_id].save_column;
	terminal_color = screens[screen_id].save_color;
	if (terminal_column == 0)
		terminal_column = PROMPT_LENGTH;
	set_cursor(terminal_row, terminal_column);
}

void	switch_screen(size_t new_screen_id)
{
	if (new_screen_id >= NUM_SCREENS || new_screen_id == current_screen)
		return ;
	
	save_screen(current_screen);
	current_screen = new_screen_id;
	load_screen(new_screen_id);

	draw_screen_index();
}

void	draw_screen_index()
{
	const char	*text = "Screen  /  ";
	size_t		start_x = VGA_WIDTH - 13;
	u8			color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

	for (size_t	index = 0; text[index]; ++index)
	{
		if (index == 7)
			terminal_buffer[start_x + index] = vga_entry('1' + current_screen, color);
		else if (index == 9)
			terminal_buffer[start_x + index] = vga_entry('0' + NUM_SCREENS, color);
		else
			terminal_buffer[start_x + index] = vga_entry(text[index], color);
	}
}

void	kernel_main(void)
{
	terminal_initialize();
	gdt_init();
	print_gdt();
	print_prompt();
	keyboard_handler_loop();
}
