#include "../includes/kernel.h"
#include "../includes/vargs.h"

int	putnbr_base(unsigned long num, int base, int uppercase)
{
	char	buffer[32];
	int		value = 0;
	char	*digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

	if (num == 0)
	{
		terminal_putchar('0');
		return (1);
	}
	while (num > 0)
	{
		buffer[value++] = digits[num % base];
		num = num / base;
	}
	for (int index = value - 1; index >= 0; --index)
		terminal_putchar(buffer[index]);

	return (value);
}

int	check_format(va_list args, char c)
{
	int	value = 0;

	switch (c)
	{
		case 'd':
		case 'i':
		{
			int	num = va_arg(args, int);
			if (num < 0)
			{
				terminal_putchar('-');
				++value;
				num = -num;
			}
				value += putnbr_base(num, 10, 0);
				break ;
		}
		
		case 'u':
			value += putnbr_base(va_arg(args, unsigned int), 10, 0);
			break ;
		
		case 'x':
			value += putnbr_base(va_arg(args, unsigned int), 16, 0);
			break ;

		case 'X':
			value += putnbr_base(va_arg(args, unsigned int), 16, 1);
			break ;

		case 'p':
		{
			terminal_write_string("0x");
			value = 2 + putnbr_base((unsigned long)va_arg(args, void*), 16, 0);
			break ;
		}

		case 's':
		{
			char *str = va_arg(args, char *);
			if (!str)
				str = ("null");
			terminal_write_string(str);
			value = ft_strlen(str);
			break ;
		}

		case 'c':
			terminal_putchar((char)va_arg(args, int));
			value = 1;
			break ;

		case '%':
			terminal_putchar('%');
			value = 1;
			break ;

		default:
			terminal_putchar('%');
			terminal_putchar(c);
			value = 2;
			break ;
	}
	return (value);
}

int	printk(const char *str, ...)
{
	va_list	args;
	int		value = 0;
	int		index = 0;

	va_start(args, str);
	if (!str)
		return (-1);

	while (str[index])
	{
		if (str[index] == '%' && str[index + 1] != '\0')
		{
			++index;
			value += check_format(args, str[index]);
		}
		else
		{
			terminal_putchar(str[index]);
			++value;
		}
		index++;
	}
	va_end(args);

	return (value);
}
