__heap_size = 0x100000;
SECTIONS
{
  . = 0xffff000000000000;
  . += 0x80000;
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

  . = . + 0x100000;

  .heap :
  {
    __heap_start = .;

    *(.heap*)

    __HeapLimit = . + __heap_size;

    . = . + __heap_size;
    
  }
  . = ALIGN(0x20000);
  _end_ = .;

}
__bss_size = (__bss_end - __bss_start) >> 3;
__dtb_addr = 0xffff000002080000;
