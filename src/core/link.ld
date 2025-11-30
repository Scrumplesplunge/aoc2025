/* Place all program output in a single program segment */
PHDRS {
  all PT_LOAD FILEHDR PHDRS;
}

/* Program entry point */
ENTRY(_start)

/* Lay out sections with minimal alignment overhead, starting at 0x10000 */
SECTIONS {
  . = 0x10000 + SIZEOF_HEADERS;
  /* Start read-only stuff immediately after the headers */
  .text ALIGN(0x1) : { *(.text*) } :all
  .rodata ALIGN(0x4) : { *(.rodata*) }
  .data ALIGN(0x4) : { *(.data*) }
  .bss ALIGN(0x4) : { *(.bss*) }
  /DISCARD/ : { *(*) }
}
