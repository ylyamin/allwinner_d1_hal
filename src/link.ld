OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)

STACK_SIZE = 0x400;

MEMORY
{
    RAM : ORIGIN = 0x40000000, LENGTH = 4M
}

ENTRY(_start)
SECTIONS
{
    . = ORIGIN(RAM);

	.text :
	{
		PROVIDE(__image_start = .);
		PROVIDE(__text_start = .);
		*(.text*)
		*(.iplt)
		*(.note.gnu.build-id)
		PROVIDE(__text_end = .);
	} > RAM

	.ksymtab ALIGN(16) :
	{
		PROVIDE(__ksymtab_start = .);
		KEEP(*(.ksymtab.text))
		PROVIDE(__ksymtab_end = .);
	} > RAM

	.romdisk ALIGN(8) :
	{
		PROVIDE(__romdisk_start = .);
		KEEP(*(.romdisk))
		PROVIDE(__romdisk_end = .);
	} > RAM

	.rodata ALIGN(8) :
	{
		PROVIDE(__rodata_start = .);
		*(.rodata*)
		*(.srodata*)
		PROVIDE(__rodata_end = .);
	} > RAM

	.data ALIGN(8) :
	{
		PROVIDE(__data_start = .);
		PROVIDE(__global_pointer$ = . + 0x800);
		*(.sdata*)
		*(.data*)
		. = ALIGN(8);
  		PROVIDE(__data_end = .);
		PROVIDE(__image_end = .);
	} > RAM

	.bss ALIGN(8) (NOLOAD) :
	{
		PROVIDE(__bss_start = .);
		*(.bss*)
		*(.sbss*)
		*(COMMON)
		. = ALIGN(8);
		PROVIDE(__bss_end = .);
	} > RAM

	.stack ALIGN(16) (NOLOAD) :
	{
		PROVIDE(__stack_start = .);
		. += STACK_SIZE;
		. = ALIGN(16);
		PROVIDE(__stack_end = .);
	} > RAM

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
