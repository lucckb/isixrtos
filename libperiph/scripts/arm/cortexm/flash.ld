ENTRY(_exceptions_vectors)
EXTERN(_exceptions_vectors)
ASSERT(DEFINED(_exceptions_vectors), "Error no exceptions vectors entries")

/* Sections Definitions */
SECTIONS
{
    /* for Cortex devices, the beginning of the startup 
	 * code is stored in the .isr_vector section, which goes to FLASH */
    .isr_vector :
    {
		. = ALIGN(4);
        KEEP(*(.isr_vector))            /* Startup code */
		. = ALIGN(4);
    } >FLASH
 
     /* .ARM.exidx is sorted, so has to go in its own output section.  */
	__exidx_start = .;
	.ARM.exidx : ALIGN(4)
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} >FLASH
    __exidx_end = .;
	
 	.eh_frame_hdr : ALIGN(4)
	{
		KEEP (*(.eh_frame_hdr))
	} >FLASH
	.eh_frame : ALIGN(4)
	{
		KEEP (*(.eh_frame))
	} >FLASH
	
    /* the program code is stored in the .text section, which goes to Flash  */
    .text :
    {
	    . = ALIGN(4);
        *(.text)                   /* remaining code */
        *(.text.*)                   /* remaining code */
	    . = ALIGN(4);
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)
	    . = ALIGN(4);
        *(.glue_7)
        *(.glue_7t)
	    . = ALIGN(4);
        *(.gnu.linkonce.t.*)
        *(.gnu.linkonce.r.*)
	    . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
	    . = ALIGN(4);
        *(.gcc_except_table)
	    . = ALIGN(4);
        *(.vfp11_veneer)
       	.  = ALIGN(4);
		KEEP(*(.init));
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP(*(.preinit_array));
		. = ALIGN(4);
		__preinit_array_end = .;
		__init_array_start = .;
		KEEP(*(SORT(.init_array.*)));
		. = ALIGN(4);
		KEEP(*(.init_array));
		. = ALIGN(4);
		__init_array_end = .;
		KEEP(*(.fini));
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP(*(.fini_array));
		. = ALIGN(4);
		KEEP(*(SORT(.fini_array.*)));
		. = ALIGN(4);
		__fini_array_end = .;
   	 _etext = .;
	    /* This is used by the startup in order to initialize the .data secion */
   	 _sidata = _etext;
    } >FLASH
    
	/** System stack size on ram start on stm32 CCM is mapped fist */
	.stack (NOLOAD):
	{
		. += _sys_stack_size;
		. = ALIGN(8);
		_estack = .;
	} > RAM
    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data : AT ( _sidata )
    {
	    . = ALIGN(8);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        *(.data)
        *(.data.*)
		*(.gnu.linkonce.d.*)
	    . = ALIGN(8);
	    /* This is used by the startup in order to initialize the .data secion */
   	 _edata = . ;
    } >RAM
    
    

    /* This is the uninitialized data section */
    .bss (NOLOAD):
    {
	    . = ALIGN(8);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        
	    . = ALIGN(8);
	    /* This is used by the startup in order to initialize the .bss secion */
   	 _ebss = . ;
    } >RAM
    
    .heap (NOLOAD):
    {
        . = ALIGN(8);
        _heap_start = .;
        . = ORIGIN(RAM) + LENGTH(RAM);
        _heap_end = .;
    } > RAM
    
    PROVIDE( __heap_start = _heap_start );
    PROVIDE( __heap_end = _heap_end );
   
	/* <WAFINSERT> Additional extra sections **/
    
    /* after that it's only debugging information. */
    
    /* remove the debugging information from the standard libraries */
    /DISCARD/ :
    {
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
	}

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
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
    
    note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
	.ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
	/DISCARD/ : { *(.note.GNU-stack) }
}


