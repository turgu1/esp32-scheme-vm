# esp32-scheme-vm

A Scheme interpreter to be used both on a linux based host and on an ESP32 IOT processor.

**__Very __** early piece of code. This is work in progress and is not ready for use.

This is a port of the PicoBit Scheme system you can find at the following
[location](https://github.com/stamourv/picobit).

The ESP32 processor provides an environment that requires a lot of changes to
the way the scheme vm operate. As such, I decided to develop a new interpreter
from scratch, using as much as possible from the PicoBit vm implementation.

Another aspect was the relocation of the ram heap to index 0 such that we
minimize processing of addresses calculation. This requires the relocation of
ROM space and small numbers/booleans constants in the higher portion of the
address space.

RAM and ROM data spaces are addressed with indexes in separate vectors. The
address space is divided in three zone:

  * 0x0000 - 0xBFFF: RAM Space
  * 0xC000 - 0xFDFF: ROM Space
  * 0xFE00 - 0xFFFF: Coded small ints, true, false and ()

Each ROM and RAM data cell is composed of 5 bytes. Please look at file
inc/vm-arch.h for details regarding the vm architecture. This address spaces
architecture is different than the original PicoBit. Small changes were
required to the compiler in support of this setup.

The code is also addressed through a separate vector. Code addresses start at
0x0000 and is byte addressable.

esp32-scheme-vm is released under the GPLv3.

Guy Turcotte  
December 2017

## Todo for the interpreter

1. As First priorities

  * Test test Test **All Scheme tests completed, Unit tests at 50%**
  * Implement bignums (no limit integers) **OK**

2. Second priorities

  * Load mechanism through the NET
  * Implement 32 bits fixnums
  * Integration on the ESP32 for IOT I/O and networking
  * Documentation **At 30%**

## Todo for the compiler

1. As first priorities to get something working with the interpreter

  * Cells formatting on 5 bytes on constants rom-space, not 4 **OK**
  * Little-Endian **OK**
  * ROM Heap space (for constants) start at virtual address 0xC000 **OK**
  * Code address space starts at 0x0000, not x8000 **OK**
  * Fixnums are 32 bits, not 24
  * Change coding of small Ints, FALSE, TRUE and () **OK**

2. Second priorities

  * Enlarge the global space
  * Enlarge the ROM heap space
  * Add version numbering in code (Major + Minor numbers)

# Original PicoBit Readme file

This is a very small Scheme system designed to run on small
microcontrollers, currently PIC18 and ARM.

It consists of

* a bytecode compiler which compiles Scheme source code to bytecode.
  The bytecode compiler is run using Racket, usually on a
  PC/workstation.

* a bytecode interpreter which can be either compiled to run on a
  microcontroller, or to be run on some common operating systems, at
  least GNU/Linux (Windows support hasn't been tested in a while,
  though, and especially the networking part might not work out of the
  box). It is written in (mostly) portable C.


## USAGE:

1. Install Racket (needed to run the bytecode compiler)
     racket-lang.org

2. Build the VM:
     make
 This will build the VM with the default configuration (workstation,
 no deubgging). To configure the VM for another architecture, or with
 different options, run
     make help
 from the `vm' directory.

Note: gawk is required to build the VM.

2. Compile a Scheme program:
     ./picobit prog.scm

3. Run the resulting program:
     ./picobit-vm prog.hex

Note: The `p' script is a shortcut for compiling and running programs:
     ./p prog.scm
 is equivalent to
     ./picobit prog.scm ; ./picobit-vm prog.hex


## SEE ALSO:

  * A paper describing PICOBIT has been presented to IFL 2009:
    http://www.ccs.neu.edu/home/stamourv/papers/picobit.pdf
    Slides from the presentation:
    http://www.ccs.neu.edu/home/stamourv/slides/picobit-ifl09.pdf

  * S3 (Small Scheme Stack) : A Scheme TCP/IP Stack Targeting Small
    Embedded Applications
    http://www.ccs.neu.edu/home/stamourv/papers/s3.pdf
    Slides from the presentation:
    http://www.ccs.neu.edu/home/stamourv/slides/s3-sw08.pdf

 PICOBIT is a descendant of the BIT and PICBIT systems. You can find
 papers describing these systems at:
    http://w3.ift.ulaval.ca/~dadub100/


## HISTORY:

 Marc Feeley originally wrote PICOBIT around 2006.
 Vincent St-Amour took over development in 2008.
 Jim Shargo worked on a port to Racket in 2011.
 The Racket port was completed in June 2011.
 Peter Zotov (whitequark) ported PICOBIT to ARM in August 2011.

## LICENCE:

 PICOBIT is released under the GPLv3.
