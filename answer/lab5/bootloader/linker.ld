ENTRY(_start)
SECTIONS
{
  . = 0x60000;
  _stext = .;
  .text : { 
    *(.text.relo)
    _bl_entry = .;
    *(.text.boot)
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

  _blsize = _ebss - _stext;
}
