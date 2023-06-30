_skernel = 0x80000;
_ekernel = 0x400000;


ENTRY(_start)
SECTIONS 
{
  . = _skernel;
  
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

  . = _ekernel;
  
}