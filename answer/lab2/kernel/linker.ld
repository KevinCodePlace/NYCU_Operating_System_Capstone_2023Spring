ENTRY(_start)

SECTIONS 
{
  . = 0x80000;
  _stext = .;
  .text : { 
    *(.text.kernel)
    *(.text) 
    *(.rodata) 
  }
  . = ALIGN(0x1000);
  _etext = .;

  _sdata = .;
  .data : { *(.data) }
  . = ALIGN(0x1000);
  _edata = .;

  
  _sbss = .;
  .bss : { *(.bss*) } 
  . = ALIGN(0x1000);
  _ebss = .;  

}