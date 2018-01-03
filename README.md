# esp32-scheme-vm

A Scheme interpreter to be used both on a linux based host and on an ESP32 IOT processor.

**__Very very very__** early piece of code. This is work in progress and is not ready for use.

This is a port of the PicoBit Scheme system you can find at the following
[location](https://github.com/stamourv/picobit).

The ESP32 processor provides an environment that requires a lot of changes to
the way the scheme vm operate. As such, I decided to develop a new interpreter
from scratch, using as much as possible from the PicoBit vm implementation.

Another aspect was the relocation of the ram heap to index 0 such that we
minimize processing of addresses calculation. This requires the relocation of
ROM space and small numbers/booleans constants in the higher portion of the
ddress space.

esp32-scheme-vm is released under the GPLv3.

Guy Turcotte  
December 2017

## Todo for the interpreter

1. As First priorities

  * Test test Test
  * Implement bignums

2. Second priorities

  * Load mechanism through the NET
  * Integration on the ESP32 for IOT I/O and networking

## Todo for the compiler

1. As first priorities to get something working with the interpreter

  * Cells formating on 5 bytes on constants rom-space, not 4 **OK**
  * Little-Endian **OK**
  * ROM Heap space (for constants) start at virtual address 0xC000 **OK**
  * Code address space starts at 0, not x8000 **OK**
  * Fixnums are 32 bits, not 24
  * Change coding of small Ints, FALSE, TRUE and () **OK**

2. Second priorities

  * Enlarge the global space
  * Enlarge the ROM heap space
  * Add version numbering in code (Major + Minor numbers)
