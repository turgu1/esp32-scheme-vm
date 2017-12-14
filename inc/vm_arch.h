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
    VM Registers
    Machine instructions

 */

/** Cells Definitions.

  Every cell is 40 bits long (5 bytes). To get good memory compaction, the following
  option must be user for gcc:

     -fpack-struct=1

  On the ESP32, alignement doesn't seems to have any issue on the performance
  of the code. At least sequential RAM access runs at the same speed even when 16 bits words
  are accessed. 

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

      A constant string is a null terminated string located in the ROM space, in the
      cstring pool.

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

// Easy enoough: all pointer elements in cells are 16 bit long. So we define here
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

/** VM Registers.

 free_cells     pointer on free cells
 environments   pointer on environments. The first one is from de code (Read only)
 continuations  pointer on continuations

 rom_heap       pointer on the rom_heap
 rom_heap_size  length of the rom_heap in cell count

 ram_heap       pointer on the ram_heap
 ram_heap_size  length of the ram_heap in cell count

 */

#define NIL  ((cell_p) 0xFFFF)

PUBLIC cell_p root;

PUBLIC void init_vm_arch();

#undef PUBLIC
#endif

