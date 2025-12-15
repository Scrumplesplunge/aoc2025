# Advent of Code 2025

Minimal-dependency solutions to Advent of Code 2025. Solutions are written in
plain C23, without use of the standard library or any other library that isn't
part of this repo. I/O is handled with hand-rolled inline assembly snippets for
the `read`, `write`, and `exit` syscalls on Linux.

Solutions are compiled with various interesting properties:

  * Compiled for the x32 ABI, allowing for full use of x86-64 instructions but
    smaller binaries and a lower memory footprint due to 32-bit pointers.
  * Linked with a custom linker script which prioritizes tiny executables above
    all else. As a result, most days compile to elf executables under 1KB.
