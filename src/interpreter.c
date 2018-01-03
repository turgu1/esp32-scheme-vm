#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"
#include "testing.h"

#define INTERPRETER 1
#include "interpreter.h"

#include "gen.primitives.h"

#define NEXT_BYTE *pc.c++
#define NEXT_SHORT *pc.s++

/** tos().

  Returns the cons in front of the stack. Get rid of
  it from the stack.
 */

cell_p tos()
{
  cell_p p = env;

  if (env == NIL) {
    FATAL("tos", "Environment exhausted");
  }
  else {
    env = RAM_GET_CDR(env);
  }

  return p;
}

/** prepare_arguments().

  Prepares reg1 for the following cases:

  1. Fixed number of arguments: reg1 will point on the
     lexical environment

  2. Variable number of arguments: reg1 will point on a cons
     which car is the list of remaining arguments and the cdr is the
     lexical environment.

  entry is updated to point to the first executable instruction
        of the procedure.

  The stack must contains arguments and the closure for the call
 */

uint8_t prepare_arguments(int8_t nbr_args)
{
  // Retrieve closure from stack, keep the cons for later
  reg2 = tos();
  reg1 = RAM_GET_CAR(reg2);

  if (IN_RAM(reg1)) {
    if (RAM_IS_CLOSURE(reg1)) {
      entry = RAM_GET_CLOSURE_ENTRY_POINT(reg1);
    }
    else {
      FATAL_MSG("prepare_arguments: Expected closure cell at %d.", reg1);
    }
  }
  else {
    FATAL_MSG("prepare_arguments: Closure on TOS not in RAM: %d.", reg1);
  }

  // Retrieve number of arguments expected in the procedure entry point
  uint8_t nbr_params = *(program + entry++);

  if (reg1 != NIL) {
    // reg1 is the closure definitions
    reg1 = RAM_GET_CLOSURE_ENV(reg1); // retrieve the environment
  }

  if ((nbr_params & 0x80) == 0) {
    if (nbr_args != nbr_params) {
      ERROR_MSG("prepare_arguments: Wrong number of arguments (%d vs %d).", nbr_args, nbr_params);
    }
  }
  else {
    // if nbr_params is 1st complement, the function accepts a dynamic
    // number of parameters
    nbr_params = ~nbr_params;

    // nbr_params is the number of fixed parameters.

    if (nbr_args < nbr_params) {
      ERROR("prepare_arguments", "Wrong number of arguments");
    }

    reg3 = NIL;

    // The loop is optimized to reuse the stack cons cells instead
    // of freeing and reallocating a cons. Save some garbage collecting job...
    while (nbr_args-- > nbr_params) {
      reg4 = tos();
      RAM_SET_CDR(reg4, reg3);
      reg3 = reg4;
      //reg3 = new_pair(pop(), reg3);
    }
    RAM_SET_CDR(reg2, reg1);
    RAM_SET_CAR(reg2, reg3);
    reg1 = reg2;
    reg3 = reg4 = NIL;
    nbr_args++;
  }

  reg2 = NIL;
  return nbr_args;
}

void save_cont()
{
  cont = new_cont(cont, new_closure(env, pc.c - program));
}

/** build_environment.

  This method complete the preparation of the argument list for the
  procedure to be called. The arguments are put in front of the
  content of reg1 as list. See the prepare_arguments for the preparation
  to be done bedore calling this function.

 */

void build_environment(uint8_t nbr_args)
{
  // The loop is optimized to reuse the stack cons cells instead
  // of freeing and reallocating a cons. Save some garbage collecting job...
  while (nbr_args--) {
    reg2 = tos();
    RAM_SET_CDR(reg2, reg1);
    reg1 = reg2;
    //reg1 = new_pair(pop(), reg1)
  }
  reg2 = NIL;
}

void interpreter()
{
  pc.c = program + (program[2] * 5) + 4;

  for (;;) {
    #if TRACING
      last_pc = pc;
    #endif
    uint8_t instr = NEXT_BYTE;

    switch (instr & 0xF0) {
      case LDCS1 :
      case LDCS2 :
        reg1 = instr & 0x1F;
        TRACE("  LDCS %d\n", reg1);
        if (reg1 < 3) {
          env = new_pair(reg1 + 0xFFFD, env);
        }
        else {
          env = new_pair((reg1 - 3) + 0xFE00, env);
        }
        reg1 = NIL;
        break;

      case LDSTK1 :
      case LDSTK2 :
        reg2 = instr & 0x1F;
        TRACE("  LDSTK %d\n", reg2);

        reg1 = env;
        while (reg2-- && (reg1 != NIL)) {
          reg1 = RAM_GET_CDR(reg1);
        }
        env = new_pair(reg1 == NIL ? NIL : RAM_GET_CAR(reg1), env);
        reg1 = reg2 = NIL;
        break;

      case LDS :
        reg2 = instr & 0x0F;
        TRACE("  LDS %d\n", reg2);
        reg1 = GLOBAL_GET(reg2);
        env = new_pair(reg1, env);
        reg1 = reg2 = NIL;
        break;

      case STS :
        reg2 = instr & 0x0F;
        TRACE("  STS %d\n", reg2);
        GLOBAL_SET(reg2, pop());
        reg2 = NIL;
        break;

      case CALLC :  // Call with closure on TOS
        reg2 = instr & 0x0F;
        TRACE("  CALLC %d\n", reg2);
        build_environment(prepare_arguments(reg2));
        save_cont();

        env = reg1;
        pc.c = program + entry;
        reg1 = reg2 = NIL;
        break;

      case JUMPC :
        reg2 = instr & 0x0F;
        TRACE("  JUMPC %d\n", reg2);
        build_environment(prepare_arguments(reg2));

        env = reg1;
        pc.c = program + entry;
        reg1 = reg2 = NIL;
        break;

      case JUMPS :
        entry = (pc.c - program) + (instr & 0x0F);
        TRACE("  JUMPS %d\n", entry);

        reg1 = NIL;
        build_environment(*(program + entry++));

        env = reg1;
        pc.c = program + entry;

        reg1 = NIL;
        break;

      case BRSF :
        TRACE("  BRSF %d\n", instr & 0x0F);
        if (pop() == FALSE) {
          pc.c += (instr & 0x0F);
        }
        break;

      case LDC :
        reg1 = ((instr & 0x0F) << 8) + NEXT_BYTE;
        TRACE("  LDC %d\n", reg1);
        if (reg1 < 3) {
          env = new_pair(reg1 += 0xFFFD, env);
        }
        else if (reg1 < 260) {
          env = new_pair((reg1 - 3) + 0xFE00, env);
        }
        else {
          env = new_pair((reg1 - 260) + 0xC000, env);
        }
        reg1 = NIL;
        break;

      case CALL: //  Call top-level procedure
        switch (instr) {
          case CALL :
            entry = NEXT_SHORT;
            TRACE("  CALL %d\n", entry);

            reg1 = NIL;
            build_environment(*(program + entry++));
            save_cont();

            env = reg1;
            pc.c = program + entry;

            reg1 = NIL;
            break;

          case JUMP : // Jump to top-level procedure
            entry = NEXT_SHORT;
            TRACE("  JUMP %d\n", entry);

            reg1 = NIL;
            build_environment(*(program + entry++));

            env = reg1;
            pc.c = program + entry;

            reg1 = NIL;
            break;

          case BR :
            entry = NEXT_SHORT;
            TRACE("  BR %d\n", entry);
            pc.c = entry + program;
            break;

          case BRF :
            entry = NEXT_SHORT;
            TRACE("  BRF %d\n", entry);
            if (pop() == FALSE) {
              pc.c = entry + program;
            }
            break;

          case CLOS :
            entry = NEXT_SHORT;
            TRACE("  CLOS %d\n", entry);

            reg2 = tos();
            reg3 = RAM_GET_CAR(reg2); // env
            reg1 = new_closure(reg3, entry);

            RAM_SET_CDR(reg2, env);  // Already set, but anyway...
            RAM_SET_CAR(reg2, reg1);

            env = reg2;
            reg1 = reg2 = reg3 = NIL;
            break;

          case CALLR :
            entry = NEXT_BYTE;
            entry = (pc.c - program) + entry - 128;
            TRACE("  CALLR %d\n", entry);
            reg1 = NIL;
            build_environment(*(program + entry++));
            save_cont();
            env = reg1;
            pc.c = program + entry;
            reg1 = NIL;
            break;

          case JUMPR :
            entry = NEXT_BYTE;
            entry = (pc.c - program) + entry - 128;
            TRACE("  JUMPR %d\n", entry);
            reg1 = NIL;
            build_environment(*(program + entry++));
            env = reg1;
            pc.c = program + entry;
            reg1 = NIL;
            break;

          case BRR :
            entry = NEXT_BYTE;
            entry =  (pc.c - program) + entry - 128;
            TRACE("  BRR %d\n", entry);
            pc.c = program + entry;
            break;

          case BRRF :
            entry = NEXT_BYTE;
            entry =  (pc.c - program) + entry - 128;
            TRACE("  BRRF %d\n", entry);
            if (pop() == FALSE) {
              pc.c = program + entry;
            }
            break;

          case CLOSR :
            entry = NEXT_BYTE ;
            entry =  (pc.c - program) + entry - 128;
            TRACE("  CLOSR %d\n", entry);

            reg2 = tos();
            reg3 = RAM_GET_CAR(reg2); // env
            reg1 = new_closure(reg3, entry);

            RAM_SET_CDR(reg2, env);  // Already set, but anyway...
            RAM_SET_CAR(reg2, reg1);

            env = reg2;
            reg1 = reg2 = reg3 = NIL;
            break;

          case LD  :
            reg2 = NEXT_BYTE;
            TRACE("  LD %d\n", reg2);
            reg1 = GLOBAL_GET(reg2);
            env = new_pair(reg1, env);
            reg1 = reg2 = NIL;
            break;

          case ST :
            reg2 = NEXT_BYTE;
            TRACE("  ST %d\n", reg2);
            GLOBAL_SET(reg2, pop());
            break;
        }
        break;
      #ifndef NO_PRIMITIVE_EXPAND
        #include "gen.dispatch.h"
      #endif
    }
  }
}


#if TESTS
void interpreter_tests()
{
  TESTM("interpreter");

}
#endif
