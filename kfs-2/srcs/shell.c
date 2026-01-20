/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumugot <lumugot@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 12:13:07 by lumugot           #+#    #+#             */
/*   Updated: 2026/01/20 12:58:20 by lumugot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"
#include "../includes/io.h"
#include "../includes/gdt.h"

int		ft_strncmp(const char *s1, const char *s2, size_t len)
{
	size_t	index = 0;
	
	while (index < len && s1[index] && s2[index] && s1[index] == s2[index])
		index++;
	if (index == len)
		return (0);
	return ((unsigned char)s1[index] - (unsigned char)s2[index]);
}

size_t	get_cmd(const char *cmd)
{
	size_t	index = 0;

	while (cmd[index] && cmd[index] != ' ')
		index++;
	return (index);
}

void	execute_command(const char *cmd)
{
	size_t	len;

	if (!cmd || !*cmd)
		return ;
		
	len = get_cmd(cmd);

	if (len == 4 && ft_strncmp(cmd,	"help", 4) == 0)
	{
		printk("Commands:\n");
		printk("help   - show this message\n");
		printk("clear  - clear screen\n");
		printk("reboot - reboot machine\n");
		printk("halt   - stop cpu\n");
		printk("gdt    - print gdt\n");
		printk(".....  - print easter egg\n");
	}
	
	else if (len == 5 && ft_strncmp(cmd, "clear", 5) == 0)
		terminal_clear_screen();

	else if (len == 6 && ft_strncmp(cmd, "reboot", 6) == 0)
		outb(0x64, 0xFE);

	else if (len == 4 && ft_strncmp(cmd, "halt", 4) == 0)
		asm volatile ("cli; hlt");
	
	else if (len == 3 && ft_strncmp(cmd, "gdt", 3) == 0)
		print_gdt();

	else if (len == 5 && ft_strncmp(cmd, "stack", 5) == 0)
		print_stack();
}
