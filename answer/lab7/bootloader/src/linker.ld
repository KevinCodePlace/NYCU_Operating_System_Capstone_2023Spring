__heap_size = 0x400;
SECTIONS
{
  . = 0x80000;
  .relocate :
  {
    KEEP(*(.text.relocate))
    *(.text.relocator)
    . = ALIGN(8);
  }
  . = ALIGN(4096);

  _begin_ = .;
  .text :
  {
    KEEP(*(.text.boot))
    *(.text)
  }
  . = ALIGN(4096);

  .data :
  {
    *(.data)
  }
  . = ALIGN(4096);

  .bss (NOLOAD) :
  {
    __bss_start = .;

    *(.bss)

    __bss_end = .;
  }
  . = ALIGN(4096);
  _end_ = .;

  .heap (NOLOAD):
  {
    . = . + 8;
    __heap_start = .;
    *(.heap*)
    __HeapLimit = . + __heap_size;
  }

}

__bss_size = (__bss_end - __bss_start) >> 3;
__relocation = 0x01080000;
__dtb_addr = 0x02080000;
