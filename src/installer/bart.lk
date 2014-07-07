OUTPUT_FORMAT("elf32-sh")

SECTIONS {		
    . = 0x06004000;
    .text : { *(.text) }
    .tors : {
       ___ctors = . ;
       *(.ctors)
       ___ctors_end = . ;
       ___dtors = . ;
       *(.dtors)
       ___dtors_end = . ;
    }    
    /* Anything rewritable should be in ram */
    .data : { *(.data) }
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = .;
    _end = .;   
}