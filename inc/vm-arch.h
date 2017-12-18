#ifndef VM_ARCH_H
#define VM_ARCH_H

#ifdef VM_ARCH
# define PUBLIC
#else
# define PUBLIC extern
#endif

/** VM Architecture Definition.

  The Virtual Machine definition is composed of the following elements:

    Cells definitions
    Globals
    Standard constants
    VM Registers
    Machine instructions

 */

/** Cells Definitions.

  Every cell is 40 bits long (5 bytes). To get good memory compaction, the following
  option must be user for gcc:

     -fpack-struct=1

  On the ESP32, alignement doesn't seems to have any issue on the performance
  of the code. At least sequential RAM access runs at the same speed even when 16 bits words
  are accessed, aligned or not.

  ToDo: Random access remains to be checked

  The address space of the heap cannot go beyond (64k * 5). To address 128K cells,
  we need 16 bits (2ˆ16 = 64k).

  - The 2 first bit are 0
  - The 4 following bits are the type of a cell
  - The 2 following bits are the GC bits
  - The rest of the cell depends of the type of cell

  We define the following cells:

   cons

      CAR and CDR are indexes in the heap

      +----+------+----+---------------+----------------+
      | 00 | 0000 | GC |      CAR      |       CDR      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   continuation

      +----+------+----+---------------+----------------+
      | 00 | 0001 | GC |    PARENT     |   CLOSURE IDX  |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   closure

      +----+------+----+---------------+----------------+
      | 00 | 0010 | GC |   CODE PTR    |   ENVIRONMENT  |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   fixnum

      +----+------+----+--------------------------------+
      | 00 | 1000 | GC |             VALUE              |
      +----+------+----+--------------------------------+
         2     4     2                 32

   bignum

      A Bignum is composed of a list of 16 bits signed numerical parts, least
      significant portion is first.

      +----+------+----+---------------+----------------+
      | 00 | 1001 | GC |    NUM PAR    |      NEXT      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   string

      A string is a list of 2 chars cells and is located in RAM in the heap. If it is composed of
      an odd numner of chars, the last one is null. A zero length string
      will have the two chars = 0

      +----+------+----+---------------+----------------+
      | 00 | 1010 | GC |   2  CHARS    |      NEXT      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

  cstring

      A string constant is a null terminated string located in the ROM space,
      in the cstring pool.

      +----+------+----+---------------+----------------+
      | 00 | 1011 | GC |  CHARS LST    |       0        |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   vector

      Lenght is in bytes (max 64k). To content the index
      (4 bytes aligned) in the vector space.

      +----+------+----+---------------+----------------+
      | 00 | 1100 | GC |    LENGTH     |   TO CONTENT   |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   symbol

      The Symbol table is in the rom area. The char list is an index (byte index in
      the constant string pool) to a null terminated string containing the human
      representation of the symbol. Next point to the next synbol cell in the
      symbol table. The chars list pointer in unique for each symbol.

      +----+------+----+---------------+----------------+
      | 00 | 1101 | GC |   CHARS LST   |       NEXT     |
      +----+------+----+---------------+----------------+
         2     4     2        16               16


 */

#define         CONS_TYPE   0
#define CONTINUATION_TYPE   1
#define      CLOSURE_TYPE   2
#define       FIXNUM_TYPE   8
#define       BIGNUM_TYPE   9
#define       STRING_TYPE  10
#define      CSTRING_TYPE  11
#define       VECTOR_TYPE  12
#define       SYMBOL_TYPE  13

#define         ATOM_MASK   8

// Easy enough: all pointer elements in cells are 16 bit long. So we define here
// the various pointer types as uint16...

typedef uint16_t IDX;

typedef IDX cell_p;
typedef IDX vector_p;
typedef IDX code_p; // TODO: Revisit....

typedef struct {
  cell_p cdr_p;
  cell_p car_p;
} cons_part;

typedef struct {
  cell_p closure_p;
  cell_p parent_p;
} continuation_part;

typedef struct {
  cell_p environment_p;
  code_p entry_point_p;
} closure_part;

typedef struct {
  cell_p next_p;
  int32_t value;
} fixnum_part;

typedef struct {
  cell_p next_p;
  short num_part;
} bignum_part;

typedef struct {
  cell_p next_p;
  char string_part[2] ;
} string_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx ;
} cstring_part;

typedef struct {
  vector_p start_p;
  unsigned short length; // in bytes
} vector_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx;
} symbol_part;

typedef struct {
  union {
            cons_part cons;
    continuation_part continuation;
         closure_part closure;
          fixnum_part fixnum;
          bignum_part bignum;
          string_part string;
         cstring_part cstring;
          vector_part vector;
          symbol_part symbol;
  };
  unsigned int gc_mark : 1;
  unsigned int gc_flip : 1;
  unsigned int    type : 4;
  unsigned int  unused : 2;
} cell;

typedef cell * cell_ptr;

/** Globals.

  Global variables are located at the beginning of the RAM Heap. As this
  heap is allocated on cell boundaries, we are putting 2 globals per cell.

 */

/** Standard Constants.

  To mitigate the amount of generated cells, standard constants are defined
  in the address space. Addresses 0xFE00 to 0xFFFF are reserved for
  predefined constants as follow:

  0xFE00 .. 0xFF00 : -1 to 255
  0xFFFD           : #f
  0xFFFE           : #t
  0xFFFF           : NIL

  For the LDCS instruction, values are coded on 5 bits as follow:

  +---------+----------+
  |   ccccc |  result  |
  +---------+----------+
  | 0 .. 28 | -1 .. 27 |
  |      29 | #f       |
  |      30 | #t       |
  |      31 | NIL      |
  +---------+----------+

  For the LDC instruction , values are coded on 9 bits as follow:

  +-----------+------------+
  | ccccccccc | result     |
  +===========+============+
  |  0 .. 256 | -1 .. 255  |
  +-----------+------------+
*/

#define TRUE  ((cell_p) 0xFFF0)
#define FALSE ((cell_p) 0xFFF1)
#define NIL   ((cell_p) 0xFFFF)
#define DECODE_CONSTANT(p) (((uint32)(p & 0xFE00)) - 1)

// n is expected to be -1 <= n <= 255
#define ENCODE_CONSTANT(n) ((n + 1) | 0xFE00)

/** VM Registers.

 env          pointer on environments. The first one is from de code (Read only)
 cont         pointer on continuations
 reg1 .. reg4 registers for function parameters and evaluation
 pc           program counter

 */

PUBLIC cell_p env, cont, reg1, reg2, reg3, reg4;
PUBLIC uint8_t * pc;

/** Instructions.

  LDCS    Load immediate small constant to TOS
          000ccccc

  LDSTK   Load stack to TOS
          001nnnnn

  LDS     Load global value to TOS, located at the beginning of the RAM Heap
          Space. Only the first 16 entries are accessible through this instruction.          iiii is an index in the heap space.
          0100iiii

  STS     Store TOS to global variable. Only the first 16 entries are accessible
          through this instruction. iiii is an index in the heap space.
          0101iiii

  CALLC   Call function with closure on TOS, n is the number of arguments
          0110nnnn

  JUMPC   Jump to function with closure on TOS, n is the number of arguments
          0111nnnn

  JUMPS   Jump to function at adress pc + x
          1000xxxx

  BRSF    Branch to location pc + x if TOS is false
          1001xxxx

  LDC     Load immediate pre-defined constant to TOS
          1010cccc cccccccc

  CALL    Call function at entry point a.
          10110000 aaaaaaaa aaaaaaaa

  JUMP    Jump to function at entry point a.
          10110001 aaaaaaaa aaaaaaaa

  BR      Unconditional Branch to location
          10110010 aaaaaaaa aaaaaaaa

  BRF     Branch to location if TOS is false
          10110011 aaaaaaaa aaaaaaaa

  CLOS    Build closure from entry point a.
          10110100 aaaaaaaa aaaaaaaa

  CALLR   Call function at location pc + a - 128
          10110101 aaaaaaaa

  JUMPR   Jump to function at location pc + a - 128
          10110110 aaaaaaaa

  BRR     Unconditional Branch to location pc + a - 128
          10110111 aaaaaaaa

  BRRF    Branch to location  pc + a - 128 if TOS is false
          10111000 aaaaaaaa

  CLOSR   Build closure from entry point pc + a - 128
          10111001 aaaaaaaa

  LD      Load global value to TOS, located at the beginning of the RAM Heap
          Space. iiiiiiii is an index in the heap space.
          10111110 iiiiiiii

  ST      Store TOS to global variable, located at the beginning of the RAM Heap
          Space. iiiiiiii is an index in the heap space.
          10111111 iiiiiiii
  */

#define LDCS1       ((uint8_t) 0x00)
#define LDCS2       ((uint8_t) 0x10)
#define LDSTK1      ((uint8_t) 0x20)
#define LDSTK2      ((uint8_t) 0x30)
#define LDS         ((uint8_t) 0x40)
#define STS         ((uint8_t) 0x50)
#define CALLC       ((uint8_t) 0x60)
#define JUMPC       ((uint8_t) 0x70)
#define JUMPS       ((uint8_t) 0x80)
#define BRSF        ((uint8_t) 0x90)
#define LDC         ((uint8_t) 0xA0)
#define CALL        ((uint8_t) 0xB0)
#define JUMP        ((uint8_t) 0xB1)
#define BR          ((uint8_t) 0xB2)
#define BRF         ((uint8_t) 0xB3)
#define CLOS        ((uint8_t) 0xB4)
#define CALLR       ((uint8_t) 0xB5)
#define JUMPR       ((uint8_t) 0xB6)
#define BRR         ((uint8_t) 0xB7)
#define BRRF        ((uint8_t) 0xB8)
#define CLOSR       ((uint8_t) 0xB9)
#define LD          ((uint8_t) 0xBE)
#define ST          ((uint8_t) 0xBF)

#define BUILTIN1    ((uint8_t) 0xC0)
#define BUILTIN2    ((uint8_t) 0xD0)
#define BUILTIN3    ((uint8_t) 0xE0)
#define BUILTIN4    ((uint8_t) 0xF0)

PUBLIC void init_vm_arch();

#undef PUBLIC
#endif
