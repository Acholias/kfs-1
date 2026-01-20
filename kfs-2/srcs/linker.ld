ENTRY(_start)

SECTIONS
{
	. = 0x00000800;

	.gdt ALIGN(8) :
	{
		_gdt_start = .;
		. = . + 56;
		_gdt_end = .;
	}

	. = 0x00100000;

	.multiboot BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
	}
	
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	_kernel_end = .;
}
