#include "esp32-scheme-vm.h"
#include "vm-arch.h"

#define MM
#include "mm.h"

/*
  // assume all mark bits and all flag bits are 0
  procedure mark(R):
    current= R;
    prev= null;
    while true do
      // follow left pointers
      while current != null && current->markBit == 0 do
        current->markBit = 1;
        if current refers to a non-atomic object then
          next= current->left; current->left= prev;
          prev= current; current= next;
      // end of while current
      // retreat
      while prev != null && prev->flagBit == 1 do
        prev->flagBit= 0; next= prev->right;
        prev->right= current; current= prev;
        prev= next;
      // end of while previous
      if prev == null then
        return;
      // switch to right subgraph
      prev->flagBit= 1;
      next= prev->left;
      prev->left= current;
      current= prev->right;
      prev->right= next;
    // end of while true

*/

#define RAM_IS_CONS(p) (ram_heap[p].type == CONS_TYPE)
#define RAM_IS_STRING_OR_BIGNUM(p) ((ram_heap[p].type == STRING_TYPE) || (ram_heap[p].type == BIGNUM_TYPE))

/** Deutsch-Schorr-Waite Garbage Collection.
 */

PRIVATE void mark(cell_p p)
{
  cell_p current = p;
  cell_p prev    = NIL;
  cell_p next;

  while (true) {
    while ((current < ram_heap_end) && (ram_heap[current].gc_mark == 0)) {
      ram_heap[current].gc_mark = 1;
      #if STATISTICS
        used_cells_count++;
      #endif
      if (RAM_IS_CONS(current)) {
        next = ram_heap[current].cons.car_p;
        ram_heap[current].cons.car_p = prev;
        prev = current;
        current = next;
      }
      else if (RAM_IS_STRING_OR_BIGNUM(current)) {
        next = ram_heap[current].bignum.next_p;
        while (next != NIL) {
          ram_heap[next].gc_mark = 1;
          #if STATISTICS
            used_cells_count++;
          #endif
          next = ram_heap[next].bignum.next_p;
        }
      }
    }

    while ((prev < ram_heap_end) && (ram_heap[prev].gc_flip == 1)) {
      ram_heap[prev].gc_flip = 0;
      next = ram_heap[prev].cons.cdr_p;
      ram_heap[prev].cons.cdr_p = current;
      current = prev;
      prev = next;
    }

    if (prev == NIL) return;

    ram_heap[prev].gc_flip = 1;
    next = ram_heap[prev].cons.car_p;
    ram_heap[prev].cons.car_p = current;
    current = ram_heap[prev].cons.cdr_p;
    ram_heap[prev].cons.cdr_p = next;
  }
}

PRIVATE void sweep()
{
  free_cells = NIL;

  #if STATISTICS
    free_cells_count = 0;
  #endif

  cell_p   p;
  cell_ptr pp;

  for (p = reserved_cells_count, pp = ram_heap; p < ram_heap_end; p++, pp++) {
    if (pp->gc_mark == 1) {
      pp->gc_mark = 0;
    }
    else {
      pp->cons.cdr_p = free_cells;
      free_cells = p;

      #if STATISTICS
        free_cells_count++;
      #endif
    }
  }

}

PRIVATE bool check_free_list(int count)
{
  cell_p next = free_cells;

  while (next != NIL) {
    count--;
    next = ram_heap[next].cons.cdr_p;
  }

  return count == reserved_cells_count;
}

bool mm_init(uint8_t globals)
{
  globals_count = globals;
  reserved_cells_count = (globals_count + 1) >> 1;

  #ifdef COMPUTER

    if ((ram_heap = (cell_ptr) calloc(40000, sizeof(cell)))  == NULL) return false;
    ram_heap_size = 40000;

  #else // ESP32
    // Todo: Memory Initialisation code for ESP32
  #endif

  rom_heap      = NULL;
  rom_heap_size = 0;

  int i = ram_heap_size;
  i += rom_heap_size;

  if (i >  MAX_HEAP_IDX) return false;

  ram_heap_end  = ram_heap_size;
  rom_heap_end  = ram_heap_end + rom_heap_size;

  #if STATISTICS
    used_cells_count = 0;
    free_cells_count = 0;
  #endif

  for (i = 0; i < reserved_cells_count; i++) {
    ram_heap[i].type = CONS_TYPE;
    ram_heap[i].cons.car_p = FALSE;
    ram_heap[i].cons.cdr_p = FALSE;
  }

  sweep();

  if (!check_free_list(ram_heap_size)) return false;

  return true;
}

void mm_gc()
{
  #if STATISTICS
    used_cells_count = 0;
  #endif

  for (unit8_t i = 0; i < reserved_cells_count; i++) mark(i);
  mark(reg1);
  mark(reg2);
  mark(reg3);
  mark(reg4);
  mark(cont);
  mark(env);

  sweep();

  #if DEBUGGING
    if ((used_cells_count + free_cells_count + reserved_cells_count) != ram_heap_size) {
      WARNING("HEAP FRAGMENTATION\n");
    }
  #endif
}

cell_p cons(cell_p car, cell_p cdr)
{
  if (free_cells == NIL) {
    mm_gc();
    if (free_cells == NIL) {
      FATAL("MEMORY EXHAUSTED!!\n");
      #ifdef COMPUTER
        exit(1);
      #endif
      #ifdef ESP32
        // Todo: Reset the processor...
      #endif
    }
  }

  cell_p p = free_cells;
  free_cells = ram_heap[free_cells].cons.cdr_p;

  ram_heap[p].type = CONS_TYPE;
  ram_heap[p].cons.car_p = car;
  ram_heap[p].cons.cdr_p = cdr;

  return p;
}
