#ifndef INTERPRETER_H
#define INTERPRETER_H

#ifdef INTERPRETER
# define PUBLIC
#else
# define PUBLIC extern
#endif

/** Code Interpreter.

  This is the principal function of the virtual machine. Usually, it will
  loop indefinitely unless a HALT instruction is received.

  code is the memory address where the code reside. It is considered as
  a read-only portion of the memory map and is byte addresable.

 */
PUBLIC void interpreter(uint8_t * code);
