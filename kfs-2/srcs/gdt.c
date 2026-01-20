/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gdt.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lumugot <lumugot@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 22:12:41 by lumugot           #+#    #+#             */
/*   Updated: 2026/01/20 12:45:27 by lumugot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/gdt.h"
#include "../includes/kernel.h"

t_gdt_entry	*gdt = (t_gdt_entry *)GDT_BASE_ADRESS;
t_gdt_ptr	gdt_ptr;

extern void	gdt_flush(u32 gdt_ptr_addr);

void	gdt_set_gate(u32 num, u32 base, u32 limit, u8 access_byte, u8 flags)
{
	if (num >= GDT_ENTRIES_COUNT)
		return ;

	// Base du segment (adress 32 bits divisé en 3 parties)
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0XFF;

	// Setup la limite du sgment (20 bits divisé en 2 parties)
	gdt[num].limit_low = (limit & 0xFFFF);

	// Combiner les 4 bits hauts de la limite + les 4 bits de flags
	gdt[num].flags_limit_hight = ((limit >> 16) & 0xFF) | (flags & 0xF0);

	// configurer l'access byte
	gdt[num].access_byte = access_byte;
}

void	gdt_init(void)
{
	// Configuration du pointeur gdt
	gdt_ptr.limit = (sizeof(t_gdt_entry) * GDT_ENTRIES_COUNT) - 1;
	gdt_ptr.base  = (u32)gdt;

	gdt_set_gate(GDT_NULL_SEGMENT, 0, 0, 0 ,0);

	gdt_set_gate(GDT_KERNEL_CODE_SEGMENT, 0, 0xFFFFFFFF, GDT_KERNEL_CODE_ACCESS, GDT_FLAGS_32BIT);

	gdt_set_gate(GDT_KERNEL_DATA_SEGMENT, 0, 0xFFFFFFFF, GDT_KERNEL_DATA_ACCESS, GDT_FLAGS_32BIT);

	gdt_set_gate(GDT_KERNEL_STACK_SEGMENT, 0, 0xFFFFFFFF, GDT_KERNEL_DATA_ACCESS | GDT_GROW_DOWN, GDT_FLAGS_32BIT);

	gdt_set_gate(GDT_USER_CODE_SEGMENT, 0, 0xFFFFFFFF, GDT_USER_CODE_ACCESS, GDT_FLAGS_32BIT);

	gdt_set_gate(GDT_USER_DATA_SEGMENT, 0, 0xFFFFFFFF, GDT_USER_DATA_ACCESS, GDT_FLAGS_32BIT);

	gdt_set_gate(GDT_USER_STACK_SEGMENT, 0, 0xFFFFFFFF, GDT_USER_DATA_ACCESS | GDT_GROW_DOWN, GDT_FLAGS_32BIT);

	gdt_flush((u32)&gdt_ptr);

	printk("[GDT] Initialized at 0x%x with %d entries\n", GDT_BASE_ADRESS, GDT_ENTRIES_COUNT);
}

void	print_gdt(void)
{
	printk("GDT base:  0x%x\n", gdt_ptr.base);
	printk("GDT limit: 0x%x\n", gdt_ptr.limit);
}

void	print_stack(void)
{
	u32 *ebp;
	u32 *esp;

	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	asm volatile("mov %%esp, %0" : "=r"(esp));

	printk("\n=== KERNEL STACK DUMP ===\n");
	printk("Stack Pointer (ESP): 0x%x\n", (u32)esp);
	printk("Base Pointer (EBP):  0x%x\n", (u32)ebp);
	printk("Stack size used:     %d bytes\n\n", (u32)(ebp - esp) * 4);

	printk("Stack contents (16 most recent values):\n");
	printk("Address      | Offset | Value\n");
	printk("-------------|--------|----------\n");

	for (int i = 0; i < 16 && (esp + i) <= ebp; i++)
		printk("0x%x | +%-5d | 0x%x\n", (u32)(esp + i), i * 4, *(esp + i));
} 
