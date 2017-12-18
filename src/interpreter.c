#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

#define INTERPRETER 1
#include "interpreter.h"

#define NEXT_BYTE *pc++
#define NEXT_SHORT ((uint16_t) *(uint16_t *)pc++); pc += 2
#define NEXT_SHORT_NOINC ((uint16_t) *(uint16_t *)pc++)

cell_p pop()
{
  #if DEBUGGING
    if ((env != NIL) && (ram_heap[env].type != CONS_TYPE)) {
      FATAL("HEAP BROKEN!!!");
      #if COMPUTER
        exit(1);
      #endif
      #if ESP32
        // Todo: Code for ESP32
      #endif
    }
  #endif

  if (env == NIL) {
    #if DEBUGGING
      WARNING("Environment exhausted on pop()\n");
    #endif

    return NIL;
  }
  cell_p p = ram_heap[env].cons.car_p;
  env = ram_heap[env].cons.cdr_p;

  return p;
}

void interpreter(uint8_t * program)
{
  reg1 =
  reg2 =
  reg3 =
  reg4 =
  cont =
  env  = NIL;

  pc =  + ((*(program + 2) * sizeof(cell)));

  for (;;) {
    uint8_t instr = NEXT_BYTE;

    switch (instr & 0xF0) {
      case LDCS1 :
      case LDCS2 :
        reg1 = instr & 0x1F;
        TRACE("  LDCS %d\n", reg1);
        if (reg1 < 29)
          env = cons(reg1 |= 0xFE00, env);
        else
          env = cons(reg1 |= 0xFF70, env);
        reg1 = NIL;
        break;

      case LDSTK1 :
      case LDSTK2 :
        uint8_t cnt = instr & 0x1F;
        TRACE("  LDSTK %d\n", cnt);

        reg1 = env;
        while (cnt-- && (reg1 != NIL)) {
          reg1 = RAM_GET_CDR(reg1);
        }
        env = cons(reg1 == NIL ? NIL : RAM_GET_CAR(reg1), env);
        reg1 = NIL;
        break;

      case LDS :
        cell_p p = instr & 0x0F;
        TRACE("  LDS %d\n", p);
        reg1 = GLOBAL_GET(p);
        env = cons(reg1, env);
        reg1 = NIL;
        break;

      case STS :
        cell_p p = instr & 0x0F;
        TRACE("  STS %d\n", p);
        GLOBAL_SET(p, pop());
        break;

      case CALLC :
        TRACE("  CALLC\n");
        break;

      case JUMPC :
        TRACE("  JUMPC\n");
        break;

      case JUMPS :
        TRACE("  JUMPS\n");
        break;

      case BRSF :
        TRACE("  BRSF %d\n", instr & 0x0F);
        if (pop() == FALSE) {
          pc += (instr & 0x0F);
        }
        break;

      case LDC :
        reg1 = ((instr & 1) << 8) + NEXT_BYTE;
        TRACE("  LDC %d\n", reg1);
        env = cons(reg1 | 0xFE00, env);
        reg1 = NIL;
        break;

      case CALL:
        switch (instr) {
          case CALL :
            TRACE("  CALL\n");
            break;

          case JUMP :
            TRACE("  JUMP\n");
            break;

          case BR :
            uint16_t addr = NEXT_SHORT;
            TRACE("  BR %d\n", addr);
            pc = addr + program;
            break;

          case BRF :
            uint16_t addr = NEXT_SHORT;
            TRACE("  BRF %d\n", addr);
            if (pop() == FALSE) {
              pc = addr + program;
            }
            break;

          case CLOS :
            TRACE("  CLOS\n");
            break;

          case CALLR :
            TRACE("  CALLR\n");
            break;

          case JUMPR :
            TRACE("  JUMPR\n");
            break;

          case BRR :
            int16_t i = NEXT_BYTE;
            TRACE("  BRR %d\n", addr);
            pc += (i - 128);
            break;

          case BRRF :
            int16_t i = NEXT_BYTE;
            TRACE("  BRRF %d\n", addr);
            if (pop() == FALSE) {
              pc += (i - 128);
            }
            break;

          case CLOSR :
            TRACE("  CLOSR\n");
            break;

          case LD  :
            cell_p p = NEXT_BYTE;
            TRACE("  LD %d\n", p);
            reg1 = GLOBAL_GET(p);
            env = cons(reg1, env);
            reg1 = NIL;
            break;

          case ST :
            cell_p p = NEXT_BYTE;
            TRACE("  ST %d\n", p);
            GLOBAL_SET(p, pop());
            break;
        }
        break;
    }
  }
}
