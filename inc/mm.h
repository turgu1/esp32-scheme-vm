#ifndef MM_H
#define MM_H

#ifdef MM
# define PUBLIC
#else
# define PUBLIC extern
#endif

/** Memory management.

   The VM Architecture, in relation with the capability offered
   with the ESP32 requiers the following memory types:

    - RAM Heap space
    - RAM Vector space
    - ROM Heap space
    - ROM Vector space

   The RAM spaces are dynamically managed using the mm module functions
   described below. The ROM spaces are considered static and cannot be
   modified in any way. They are provided with the esp32-scheme-vm
   program in support of the various scheme structures embedded with the
   vm.

*/

/** Memory Management Initialisation.

  Allocates heap space for RAM Heap and Vector Space.
  Retrieve ROM heap and vector space addresses

 */

#ifdef MM

  /**
     RAM and ROM heaps are combined in a "virtual" heap through the indices in use.
     RAM indices start at 0 and end with the size of available contiguous allocatable
     space from the ESP-IDF. ROM indices start there up to the lenght of the ROM
     cells array.  The maximum indice to access ROM must be lower than
     65536 - 512: 0xFE00.
   */
  #define MAX_HEAP_IDX ((IDX) 0xFE00)

  PRIVATE cell_ptr ram_heap;   // read/write
  PRIVATE cell_ptr rom_heap;   // read-only

  PRIVATE cell_p free_cells;

  #if DEBUGGING
    PRIVATE uint16_t free_cells_count;
    PRIVATE uint16_t used_cells_count;
  #endif


  PRIVATE IDX ram_heap_size; // as a number of cells
  PRIVATE IDX rom_heap_size; // as a number of cells

  PRIVATE cell_p rom_heap_start;
  PRIVATE cell_p rom_heap_end;

#endif

PUBLIC uint16_t global_count;
PUBLIC uint8_t reserved_cells_count;

PUBLIC cell_p ram_heap_end;

#define RAM_GET_CAR(p) ((p < ram_heap_end) ? ram_heap[p].cons.car_p : NIL)
#define RAM_GET_CDR(p) ((p < ram_heap_end) ? ram_heap[p].cons.cdr_p : NIL)

#define RAM_SET_CAR(p, v) if (p < ram_heap_end) ram_heap[p].cons.car_p = v
#define RAM_SET_CDR(p, v) if (p < ram_heap_end) ram_heap[p].cons.cdr_p = v

#define RAM_SET_CAR_NO_TEST(p, v) ram_heap[p].cons.car_p = v
#define RAM_SET_CDR_NO_TEST(p, v) ram_heap[p].cons.cdr_p = v

#define GLOBAL_GET(i) ((i & 1) ? RAM_GET_CAR_NO_TEST(i >> 1) : RAM_GET_CDR_NO_TEST(i >> 1))
#define GLOBAL_SET(i, p) if (i & 1) RAM_SET_CAR_NO_TEST(i >> 1, p); else RAM_SET_CDR_NO_TEST(i >> 1, p)

PUBLIC bool mm_init(uint8_t globals);
PUBLIC void mm_gc();

PUBLIC cell_p cons(cell_p car, cell_p cdr);

#undef PUBLIC
#endif
