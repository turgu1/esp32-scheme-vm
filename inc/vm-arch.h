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

  Every cell is 40 bits long (5 bytes). To get proper memory compaction for the
  structure defining a cell, the following option must be used for gcc:

     -fpack-struct=1

  It is defined as a pragma in file inc/esp32-scheme-vm.h as follow:

    #pragma pack(1)

  On the ESP32, adresses misalignement of 16 and 32 bits values don't seems to
  create any issue on the performance of the code. At least sequential RAM
  access runs at the same speed even when 16 bits words are accessed, aligned
  or not.

  ToDo: Random access remains to be checked

  The address space of the virtual heap cannot go beyond (64k * 5). To address
  64K cells, we need 16 bits (2ˆ16 = 64k).

  - The 2 first bit are not used by the interpreter but are defined as
    user_1 and user_2 flags. New primitives are allowed to use them. They area
    protected by the garbage collector.
  - The 4 following bits are the type of a cell
  - The 2 following bits are the GC bits
  - The rest of the cell depends of the type of cell

  Types 0 .. 7 are reserved for cell types having car pointing at other
  cells. Types 0 .. 3 are reserved for cells having cdr pointing at other
  cells. These are to optimize mm_mask() of the gc procedure. This must
  be always respected to insure proper memory management.

  We define the following cell types:

   cons (or pair)

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
      | 00 | 0100 | GC |  ENVIRONMENT  |    CODE PTR    |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   bignum

      A Bignum is composed of a list of 16 bits signed numerical parts, least
      significant portion is first.

      +----+------+----+---------------+----------------+
      | 00 | 0101 | GC |     NEXT      |    NUM PART    |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   string

      A string is a list of chars cells and is located in RAM in the heap.
      A zero length string will have NIL into chars ptr.

      +----+------+----+---------------+----------------+
      | 00 | 0110 | GC |  CHARS LIST   |      NIL       |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

   fixnum

      +----+------+----+--------------------------------+
      | 00 | 1000 | GC |             VALUE              |
      +----+------+----+--------------------------------+
         2     4     2                 32

  cstring

      For now, no cstring are present. The following will be implemented later:

      A string constant is a null terminated string located in the ROM space,
      in the cstring pool (?).

      +----+------+----+---------------+----------------+
      | 00 | 1011 | GC |   CHARS PTR   |       0        |
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

      For now, no symbols information are present. Its address in the ROM
      constant area insure its uniqueness. A symbol as a constant will be showned
      that way:

      +----+------+----+---------------+----------------+
      | 00 | 1101 | GC |      NIL      |       NIL      |
      +----+------+----+---------------+----------------+
         2     4     2        16               16

      The following could be implemented later:

      The Symbol table is in the rom area. The char list is an index (byte index
      in the constant string pool) to a null terminated string containing the
      human representation of the symbol. Next point to the next synbol cell in
      the symbol table. The chars list pointer in unique for each symbol.

      +----+------+----+---------------+----------------+
      | 00 | 1101 | GC |   CHARS LST   |       NEXT     |
      +----+------+----+---------------+----------------+
         2     4     2        16               16


 */

#define         CONS_TYPE   0
#define CONTINUATION_TYPE   1
#define      CLOSURE_TYPE   4
#define       BIGNUM_TYPE   5
#define       STRING_TYPE   6

#define       FIXNUM_TYPE   8
#define      CSTRING_TYPE  11
#define       VECTOR_TYPE  12
#define       SYMBOL_TYPE  13

// Easy enough: all pointer elements in cells are 16 bit long. So we define here
// the various pointer types as uint16...

typedef uint16_t IDX;

typedef IDX cell_p;
typedef IDX vector_p;
typedef IDX code_p; // TODO: Revisit....

typedef struct {
  cell_p cdr_p;
  cell_p car_p;
} pair_part;

typedef struct {
  cell_p closure_p;
  cell_p parent_p;
} continuation_part;

typedef struct {
  code_p entry_point_p;
  cell_p environment_p;
} closure_part;

typedef struct {
  int32_t value;
} fixnum_part;

typedef struct {
  int16_t num_part;
  cell_p next_p;
} bignum_part;

typedef struct {
  cell_p unused ;
  cell_p chars_p;
} string_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx ;
} cstring_part;

typedef struct {
  vector_p start_p;
  uint16_t length; // in bytes
} vector_part;

typedef struct {
  cell_p next_p;
  IDX chars_idx;
} symbol_part;

typedef struct {
  union {
            pair_part cons;
    continuation_part continuation;
         closure_part closure;
          fixnum_part fixnum;
          bignum_part bignum;
          string_part string;
         cstring_part cstring;
          vector_part vector;
          symbol_part symbol;
  };
  union {
    struct {
      unsigned int gc_mark : 1;
      unsigned int gc_flip : 1;
      unsigned int    type : 4;
      unsigned int  user_1 : 1;
      unsigned int  user_2 : 1;
    };
    uint8_t bits;
  };
} cell;

typedef cell * cell_ptr;

/** Globals.

  Global variables are located at the beginning of the RAM Heap. As this
  heap is allocated on cell boundaries, we are putting 2 globals per cell.
  Global access instructions limit to 256 globals.

 */

/** Standard Constants.

  To mitigate the amount of generated cells, standard constants are defined
  in the address space. Addresses 0xFE00 to 0xFFFF are reserved for
  predefined constants as follow:

  0xFE00 .. 0xFF00 : -1 to 255
  0xFFFD           : #f
  0xFFFE           : #t
  0xFFFF           : NIL

  ROM Space constants are receiving addresses starting at 0xC000.

  For the LDCS instruction, values are coded on 5 bits as follow:

  +---------+----------+------------------+
  |   ccccc |  result  | addr mapping     |
  +---------+----------+------------------+
  |       0 |       #f | 0xFFFD           |
  |       1 |       #t | 0xFFFE           |
  |       2 |      NIL | 0xFFFF           |
  | 3 .. 31 | -1 .. 27 | 0xFE00 .. 0xFE1C |
  +---------+----------+------------------+

  For the LDC instruction , values are coded on 12 bits as follow:

  +--------------+-----------+------------------+
  | cccccccccccc |   result  | addr mapping     |
  +--------------+-----------+------------------+
  |            0 |        #f | 0xFFFD           |
  |            1 |        #t | 0xFFFE           |
  |            2 |       NIL | 0xFFFF           |
  |     3 .. 259 | -1 .. 255 | 0xFE00 .. 0xFF00 |
  | 260 ..       |   ROM IDX | 0xC000 ..        |
  +--------------+-----------+------------------+

*/

#define FALSE ((cell_p) 0xFFFD)
#define TRUE  ((cell_p) 0xFFFE)
#define NIL   ((cell_p) 0xFFFF)
#define ZERO  ((cell_p) 0xFE01)
#define NEG1  ((cell_p) 0xFE00)
#define POS1  ((cell_p) 0xFE02)

/** Instructions.

  The instructions are the same as the PICOBIT vm.

  LDCS    Load immediate small constant to TOS
          000ccccc

  LDSTK   Load stack to TOS
          001nnnnn

  LDS     Load global value to TOS, located at the beginning of the RAM Heap
          Space. Only the first 16 entries are accessible through this
          instruction. iiii is an index in the heap space.
          0100iiii

  STS     Store TOS to global variable. Only the first 16 entries are accessible
          through this instruction. iiii is an index in the heap space.
          0101iiii

  CALLC   Call procedure with closure on TOS, n is the number of arguments
          0110nnnn

  JUMPC   Jump to procedure with closure on TOS, n is the number of arguments
          0111nnnn

  JUMPS   Jump to procedure at adress pc + x
          1000xxxx

  BRSF    Branch to location pc + x if TOS is false
          1001xxxx

  LDC     Load immediate pre-defined constant to TOS
          1010cccc cccccccc

  CALL    Call top-level procedure at entry point a.
          10110000 aaaaaaaa aaaaaaaa

  JUMP    Jump to top-level procedure at entry point a.
          10110001 aaaaaaaa aaaaaaaa

  BR      Unconditional Branch to location
          10110010 aaaaaaaa aaaaaaaa

  BRF     Branch to location if TOS is false
          10110011 aaaaaaaa aaaaaaaa

  CLOS    Build closure from entry point a.
          10110100 aaaaaaaa aaaaaaaa

  CALLR   Call procedure at location pc + a - 128
          10110101 aaaaaaaa

  JUMPR   Jump to procedure at location pc + a - 128
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

#define LDCS1                      ((uint8_t) 0x00)
#define LDCS2                      ((uint8_t) 0x10)
#define LDSTK1                     ((uint8_t) 0x20)
#define LDSTK2                     ((uint8_t) 0x30)
#define LDS                        ((uint8_t) 0x40)
#define STS                        ((uint8_t) 0x50)
#define CALLC                      ((uint8_t) 0x60)
#define JUMPC                      ((uint8_t) 0x70)
#define JUMPS                      ((uint8_t) 0x80)
#define BRSF                       ((uint8_t) 0x90)
#define LDC                        ((uint8_t) 0xA0)
#define CALL                       ((uint8_t) 0xB0)
#define JUMP                       ((uint8_t) 0xB1)
#define BR                         ((uint8_t) 0xB2)
#define BRF                        ((uint8_t) 0xB3)
#define CLOS                       ((uint8_t) 0xB4)
#define CALLR                      ((uint8_t) 0xB5)
#define JUMPR                      ((uint8_t) 0xB6)
#define BRR                        ((uint8_t) 0xB7)
#define BRRF                       ((uint8_t) 0xB8)
#define CLOSR                      ((uint8_t) 0xB9)
#define LD                         ((uint8_t) 0xBE)
#define ST                         ((uint8_t) 0xBF)

#define PRIMITIVE1                 ((uint8_t) 0xC0)
#define PRIMITIVE2                 ((uint8_t) 0xD0)
#define PRIMITIVE3                 ((uint8_t) 0xE0)
#define PRIMITIVE4                 ((uint8_t) 0xF0)

#define MAX_SMALL_INT_VALUE        255
#define MIN_SMALL_INT_VALUE        -1

#define SMALL_INT_START            ((IDX) 0xFE00)
#define SMALL_INT_MAX              ((IDX) 0xFE00 + 256)
#define SMALL_INT_MASK             0x01FF
#define IS_SMALL_INT(p)            ((p >= SMALL_INT_START) && (p <= SMALL_INT_MAX))

#define SMALL_INT_VALUE(p)         (((int16_t)(p & SMALL_INT_MASK)) - 1)
#define ENCODE_SMALL_INT(v)        ((cell_p) ((v) + 1) | SMALL_INT_START)

#define ROM_START_ADDR             ((IDX) 0xC000)
#define ROM_MAX_ADDR               ((IDX) 0xFE00)
#define ROM_IDX(p)                 (p & 0x3FFF)

#define RAM_IS_PAIR(p)             (ram_heap[p].type ==         CONS_TYPE)
#define RAM_IS_CONTINUATION(p)     (ram_heap[p].type == CONTINUATION_TYPE)
#define RAM_IS_CLOSURE(p)          (ram_heap[p].type ==      CLOSURE_TYPE)
#define RAM_IS_FIXNUM(p)           (ram_heap[p].type ==       FIXNUM_TYPE)
#define RAM_IS_BIGNUM(p)           (ram_heap[p].type ==       BIGNUM_TYPE)
#define RAM_IS_STRING(p)           (ram_heap[p].type ==       STRING_TYPE)
#define RAM_IS_CSTRING(p)          (ram_heap[p].type ==      CSTRING_TYPE)
#define RAM_IS_VECTOR(p)           (ram_heap[p].type ==       VECTOR_TYPE)
#define RAM_IS_SYMBOL(p)           (ram_heap[p].type ==       SYMBOL_TYPE)
#define RAM_IS_NUMBER(p)           (RAM_IS_FIXNUM(p) || RAM_IS_BIGNUM(p))

#define ROM_IS_PAIR(p)             (rom_heap[ROM_IDX(p)].type ==         CONS_TYPE)
#define ROM_IS_CONTINUATION(p)     (rom_heap[ROM_IDX(p)].type == CONTINUATION_TYPE)
#define ROM_IS_CLOSURE(p)          (rom_heap[ROM_IDX(p)].type ==      CLOSURE_TYPE)
#define ROM_IS_FIXNUM(p)           (rom_heap[ROM_IDX(p)].type ==       FIXNUM_TYPE)
#define ROM_IS_BIGNUM(p)           (rom_heap[ROM_IDX(p)].type ==       BIGNUM_TYPE)
#define ROM_IS_STRING(p)           (rom_heap[ROM_IDX(p)].type ==       STRING_TYPE)
#define ROM_IS_CSTRING(p)          (rom_heap[ROM_IDX(p)].type ==      CSTRING_TYPE)
#define ROM_IS_VECTOR(p)           (rom_heap[ROM_IDX(p)].type ==       VECTOR_TYPE)
#define ROM_IS_SYMBOL(p)           (rom_heap[ROM_IDX(p)].type ==       SYMBOL_TYPE)
#define ROM_IS_NUMBER(p)           (ROM_IS_FIXNUM(p) || ROM_IS_BIGNUM(p))

#define IN_RAM(p)                  (p >= reserved_cells_count) && (p < ram_heap_size)
#define IN_ROM(p)                  (p >= ROM_START_ADDR) && (p < ROM_MAX_ADDR)

#define RAM_GET_TYPE(p)            ram_heap[p].type
#define RAM_SET_TYPE(p, t)         ram_heap[p].type = t

#define RAM_GET_BITS(p)            ram_heap[p].bits

#define RAM_GET_CAR(p)             ram_heap[p].cons.car_p
#define RAM_GET_CDR(p)             ram_heap[p].cons.cdr_p

#define RAM_SET_CAR(p, v)          ram_heap[p].cons.car_p = v
#define RAM_SET_CDR(p, v)          ram_heap[p].cons.cdr_p = v

#define ROM_GET_CAR(p)             rom_heap[ROM_IDX(p)].cons.car_p
#define ROM_GET_CDR(p)             rom_heap[ROM_IDX(p)].cons.cdr_p

#define RAM_IS_MARKED(p)           (ram_heap[p].gc_mark == 1)
#define RAM_IS_NOT_MARKED(p)       (ram_heap[p].gc_mark == 0)
#define RAM_SET_MARK(p)            ram_heap[p].gc_mark = 1
#define RAM_CLR_MARK(p)            ram_heap[p].gc_mark = 0

#define RAM_IS_FLIPPED(p)          (ram_heap[p].gc_flip == 1)
#define RAM_SET_FLIP(p)            ram_heap[p].gc_flip = 1
#define RAM_CLR_FLIP(p)            ram_heap[p].gc_flip = 0

#define RAM_IS_USER_1_SET(p)       (ram_heap[p].user_1 == 1)
#define RAM_SET_USER_1(p)          ram_heap[p].user_1 = 1
#define RAM_CLR_USER_1(p)          ram_heap[p].user_1 = 0

#define RAM_IS_USER_2_SET(p)       (ram_heap[p].user_2 == 1)
#define RAM_SET_USER_2(p)          ram_heap[p].user_2 = 1
#define RAM_CLR_USER_2(p)          ram_heap[p].user_2 = 0

#define HAS_NO_RIGHT_LINK(p)       ((ram_heap[p].bits & 0x30) != 0)
#define HAS_RIGHT_LINK(p)          ((ram_heap[p].bits & 0x30) == 0)
#define HAS_LEFT_LINK(p)           ((ram_heap[p].bits & 0x20) == 0)

// Fixnum

#define RAM_GET_FIXNUM_VALUE(p)    ram_heap[p].fixnum.value
#define ROM_GET_FIXNUM_VALUE(p)    rom_heap[ROM_IDX(p)].fixnum.value

#define RAM_SET_FIXNUM_VALUE(p, v) ram_heap[p].fixnum.value = v

// Bignum

#define RAM_GET_BIGNUM_VALUE(p)    ram_heap[p].bignum.num_part
#define ROM_GET_BIGNUM_VALUE(p)    rom_heap[ROM_IDX(p)].bignum.num_part

#define RAM_GET_BIGNUM_HI(p)       ram_heap[p].bignum.next_p
#define ROM_GET_BIGNUM_HI(p)       rom_heap[ROM_IDX(p)].bignum.next_p

#define RAM_SET_BIGNUM_HI(p,h)     ram_heap[p].bignum.next_p = h
#define RAM_SET_BIGNUM_VALUE(p,v)  ram_heap[p].bignum.num_part = v

// Vector

#define RAM_GET_VECTOR_LENGTH(p)   ram_heap[p].vector.length
#define ROM_GET_VECTOR_LENGTH(p)   rom_heap[ROM_IDX(p)].vector.length

#define RAM_SET_VECTOR_LENGTH(p, v) ram_heap[p].vector.length = v

#define RAM_SET_VECTOR_START(p, v) ram_heap[p].vector.start_p = v
#define RAM_GET_VECTOR_START(p)    ram_heap[p].vector.start_p
#define ROM_GET_VECTOR_START(p)    rom_heap[ROM_IDX(p)].vector.start_p

#define VECTOR_GET_LENGTH(p)       vector_heap[p].vector.length
#define VECTOR_GET_RAM_PTR(p)      vector_heap[p].vector.start_p

#define VECTOR_SET_LENGTH(p, l)    vector_heap[p].vector.length = l
#define VECTOR_SET_RAM_PTR(p, r)   vector_heap[p].vector.start_p = r

#define VECTOR_IS_USED(p)          (vector_heap[p].gc_mark == 1)
#define VECTOR_IS_FREE(p)          (vector_heap[p].gc_mark == 0)

#define VECTOR_SET_USED(p)         vector_heap[p].gc_mark = 1
#define VECTOR_SET_FREE(p)         vector_heap[p].gc_mark = 0

#define VECTOR_GET_BYTE(p, i)      *(((uint8_t *) &vector_heap[p]) + i)
#define VECTOR_SET_BYTE(p, i, b)   *(((uint8_t *) &vector_heap[p]) + i) = b

// String

#define RAM_STRING_GET_CHARS(p)    ram_heap[p].string.chars_p
#define ROM_STRING_GET_CHARS(p)    rom_heap[ROM_IDX(p)].string.chars_p

#define RAM_STRING_SET_CHARS(p, v) ram_heap[p].string.chars_p = v
#define RAM_STRING_CLR_UNUSED(p)   ram_heap[p].string.unused = 0


// Continuation

#define RAM_GET_CONT_CLOSURE(p)    ram_heap[p].continuation.closure_p
#define RAM_GET_CONT_PARENT(p)     ram_heap[p].continuation.parent_p

#define RAM_SET_CONT_CLOSURE(p, v) ram_heap[p].continuation.closure_p = v
#define RAM_SET_CONT_PARENT(p,v)   ram_heap[p].continuation.parent_p = v

// Closure

#define RAM_GET_CLOSURE_ENV(p)            ram_heap[p].closure.environment_p
#define RAM_GET_CLOSURE_ENTRY_POINT(p)    ram_heap[p].closure.entry_point_p

#define RAM_SET_CLOSURE_ENV(p, v)         ram_heap[p].closure.environment_p = v
#define RAM_SET_CLOSURE_ENTRY_POINT(p, v) ram_heap[p].closure.entry_point_p = v

// Globals

#define GLOBAL_GET(i)            ((i & 1) ? RAM_GET_CAR(i >> 1) : RAM_GET_CDR(i >> 1))
#define GLOBAL_SET(i, p)         if (i & 1) RAM_SET_CAR(i >> 1, p); else RAM_SET_CDR(i >> 1, p)

#define ENCODE_BOOL(bool_value)  (bool_value) ? TRUE : FALSE

/** VM Registers.

 env          index on environments. The first one is from de code (Read only)
 cont         index on continuations
 entry        index in code of a procedure entry point
 reg1 .. reg4 registers for function parameters and evaluation
 pc           program counter (pointer into rom code space)

 Golden rules for those who want to create their own primitives or want to
 modify the virtual machine:

 1. Never allocate new cells without having them pointed directly or indirectly
    by one of the registers (env cont reg1 .. reg4) or global variables. When
    garbage collection is fired, only the cells connected to these registers
    and the global variables are kept. All the other cells are put back to the
    free list and their content will be lost.

  2. Never used registers (env cont reg1 .. reg4) and global variables as
     scratch space for anything else than scheme cells or encoded values. The
     garbage collector could become mixed up if those registers contain values
     that are not related to the address space and encoded values managed by
     the virtual machine.

  In the preceeding paragraphs, global values are the ones generated by the
  compiler and located in the RAM heap starting at index 0 (two global
  variables per cell).

 */

PUBLIC cell_p env, cont, reg1, reg2, reg3, reg4;
PUBLIC code_p entry;
PUBLIC uint8_t * program;
PUBLIC uint16_t max_addr;
PUBLIC int32_t a1, a2, a3;

PUBLIC union {
  uint8_t  * c;
  uint16_t * s;
} pc, last_pc;

PUBLIC void vm_arch_init();

PUBLIC cell_p pop();

PUBLIC cell_p new_closure(cell_p env, code_p code);
PUBLIC cell_p new_cont(cell_p parent, cell_p closure);
PUBLIC cell_p new_pair(cell_p car, cell_p cdr);
PUBLIC cell_p new_string(cell_p chars_p);
PUBLIC cell_p new_fixnum(int32_t value);
PUBLIC cell_p new_bignum(int16_t lo, cell_p high);
PUBLIC cell_p new_vector(uint16_t length);

PUBLIC int32_t decode_int(cell_p p);
PUBLIC cell_p encode_int(int32_t val);

PUBLIC void decode_2_int_args();

PUBLIC uint16_t global_count;
PUBLIC uint8_t reserved_cells_count;

PUBLIC IDX vector_heap_size;

PUBLIC cell_ptr ram_heap;   // read/write
PUBLIC cell_p   ram_heap_end;
PUBLIC IDX      ram_heap_size; // as a number of cells

PUBLIC cell_ptr rom_heap;  // read only

PUBLIC cell_ptr vector_heap;


#undef PUBLIC
#endif
