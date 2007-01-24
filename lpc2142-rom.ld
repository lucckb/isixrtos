/*   Plik konfiguracyjny linkera dla LPC2142 
	Program umiesczony w pamieci FLASH
*/


/* Konfiguracja pamieci */

MEMORY
{
  CODE (xr) : ORIGIN = 0x00000000, LENGTH = 64K
  DATA (rw) : ORIGIN = 0x40000000, LENGTH = 16K
}


/* Definicje poszczegolnych sekcji */

SECTIONS
{

  /* Sekcja .text zawiera kod programu */
  . = 0;	
  .text :
  {
    *boot.o (.text)         /* Startup code */
    *(.text)                   /* remaining code */
    *(.text.*)
    *(.glue_7t) 
    *(.glue_7)
    *(.gnu.linkonce.t.*)
    *(.gcc_except_table)
  } >CODE

  . = ALIGN(4);

  
  /* sekcja .rodata  zawiera dane stale */
  .rodata :
  {
   *(.rodata) 
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
  } >CODE
  
  . = ALIGN(4);
	
	/* sekcja .ctors zawiera konstruktory */
	.ctors :
	{
		PROVIDE(__ctors_start__ = .);
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end__ = .);
	} >CODE
	
	. = ALIGN(4);
	/* sekcja .dtors zawiera destruktory */
	.dtors :
	{
		PROVIDE(__dtors_start__ = .); 
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		PROVIDE(__dtors_end__ = .);
	} >CODE
  
    . = ALIGN(4);

  
  _etext = . ;
  PROVIDE (etext = .);

  /* sekcja .data  zawiera dane zaincjalizowane */

  .data : AT (_etext)
  {
    _data = . ;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  } >DATA
  . = ALIGN(4);

  _edata = . ;
   PROVIDE (edata = .);

  /* .bss sekcja bss zawiera dane niezaincjalizowane */


  .bss :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >DATA
  . = ALIGN(4);
  __bss_end__ = . ;
  __bss_end__ = . ;

  _end = .;
  PROVIDE (end = .);

  /* Stabs zawiera sekcje debugera  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }

  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}
