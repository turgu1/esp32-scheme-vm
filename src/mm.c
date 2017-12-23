#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "testing.h"

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

#define RAM_IS_PAIR(p) (ram_heap[p].type == CONS_TYPE)
#define RAM_IS_STRING_OR_BIGNUM(p) ((ram_heap[p].type == STRING_TYPE) || (ram_heap[p].type == BIGNUM_TYPE))

/** Deutsch-Schorr-Waite Garbage Collection.
 */

PRIVATE void mark(cell_p p)
{
  cell_p current = p;
  cell_p prev    = NIL;
  cell_p next;

  for (;;) {
    while ((current < ram_heap_end) && (ram_heap[current].gc_mark == 0)) {
      ram_heap[current].gc_mark = 1;
      #if STATISTICS
        used_cells_count++;
      #endif
      if (RAM_IS_PAIR(current)) {
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

PRIVATE void mm_sweep()
{
  free_cells = NIL;

  #if STATISTICS
    free_cells_count = 0;
  #endif

  cell_p   p = ram_heap_end - 1;
  cell_ptr pp = &ram_heap[p];

  // Don't forget: p cannot be a negative number...
  do {
    if (pp->gc_mark == 1) {
      pp->gc_mark = 0;
    }
    else {
      if (RAM_IS_VECTOR(p)) {
        VECTOR_SET_FREE(RAM_GET_VECTOR_START(p) - 1);
      }
      pp->type = CONS_TYPE;
      pp->cons.cdr_p = free_cells;
      free_cells = p;

      #if STATISTICS
        free_cells_count++;
      #endif
    }
    pp--;

    // Last loop will have p = reserved_cells_count...
  } while (p-- > reserved_cells_count);

  // Reset mark bits in the globals area
  do {
    pp->gc_mark = 0;
    pp--;
  } while (p-- > 0); // Last loop will have p = 0
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

PRIVATE void mm_gc()
{
  #if STATISTICS
    used_cells_count = 0;
  #endif

  for (uint8_t i = 0; i < reserved_cells_count; i++) mark(i);
  mark(reg1);
  mark(reg2);
  mark(reg3);
  mark(reg4);
  mark(cont);
  mark(env);

  mm_sweep();

  #if DEBUGGING
    if ((used_cells_count + free_cells_count) != ram_heap_size) {
      WARNING_MSG(
        "mm_gc: HEAP FRAGMENTATION (heap_size: %d, total: %d)",
        ram_heap_size,
        used_cells_count + free_cells_count);
    }
  #endif
}

/**
  Vector space compaction. All allocated spaces are pushed to the start of
  the vector heap to collect all free spaces in a single batch of bytes.
  */

PRIVATE void mm_compact_vector_space ()
{
  cell_p cur = 0;
  cell_p prev = NIL;

  uint16_t cur_size;

	while (cur < vector_free_cells) {
		cur_size  = VECTOR_GET_LENGTH(cur);

    if (cur_size == 0) FATAL("mm_compact_vector_space", "Vector Heap Structure is wrong");

		if ((prev != NIL) && VECTOR_IS_FREE(prev)) { // previous block is free
			if (VECTOR_IS_FREE(cur)) {
        // current is free too, merge free spaces
				// advance cur, but prev stays in place
				cur += cur_size;
			}
      else {
        // prev is free, but not cur, move cur to start at prev
				// fix header in the object heap to point to the data's new
				// location
				RAM_SET_VECTOR_START(VECTOR_GET_RAM_PTR(cur), prev + 1);

        // Move cur vector in its new place
        //char * src = (char *) &vector_heap[cur];
        //char * dst = (char *) &vector_heap[prev];
        //int len = cur_size * sizeof(cell);
        //while (len--) *dst++ = *src++;

        memcpy(&vector_heap[prev], &vector_heap[cur], cur_size * sizeof(cell));

				VECTOR_SET_FREE(cur);

        prev += cur_size;
  			cur += cur_size;
			}
		}
    else {
			// Go to the next block, which is <size> away from cur.
			prev = cur;
			cur += cur_size;
		}
	}

	// free space is now all at the end
	vector_free_cells = (prev == NIL) ? 0 : prev;
}

cell_p mm_new_vector_cell(uint16_t length, cell_p from)
{
	// get minimum number of sizeof(cell) blocks (round to nearest sizeof(cell))
	// this includes a sizeof(cell)-byte vector space header
	length = ((length + sizeof(cell) - 1) / sizeof(cell)) + 1;

	if ((vector_heap_size - vector_free_cells) < length) {

    mm_gc ();
		mm_compact_vector_space();

		// free space too small, trigger gc
		if ((vector_heap_size - vector_free_cells) < length) { // we gc'd, but no space is big enough for the vector
			FATAL("alloc_vec_cell", "No room for vector");
		}
  }

	cell_p o = vector_free_cells;

	// advance the free pointer
	vector_free_cells += length;

	VECTOR_SET_LENGTH(o, length);
  VECTOR_SET_RAM_PTR(o, from);
  VECTOR_SET_USED(o);

	// return pointer to start of data, skipping the header
	return o + 1;
}


cell_p mm_new_ram_cell()
{
  if (free_cells == NIL) {
    mm_gc();
    if (free_cells == NIL) {
      FATAL("mm_gc", "MEMORY EXHAUSTED!!");
    }
  }

  cell_p p = free_cells;
  free_cells = RAM_GET_CDR_NO_TEST(free_cells);

  return p;
}

bool mm_init(uint8_t globals)
{
  reg1 =
  reg2 =
  reg3 =
  reg4 =
  cont =
  env  = NIL;

  global_count = globals;
  reserved_cells_count = (global_count + 1) >> 1;

  #ifdef COMPUTER

    if ((ram_heap = (cell_ptr) calloc(RAM_HEAP_ALLOCATED, sizeof(cell)))  == NULL) return false;
    ram_heap_size = 40000;

    if ((vector_heap = (cell_ptr) calloc(VECTOR_HEAP_ALLOCATED, sizeof(cell))) == NULL) return false;
    vector_heap_size = 30000;

  #else // ESP32
    // Todo: Memory Initialisation code for ESP32
  #endif

  rom_heap      = NULL;

  vector_free_cells = 0;

  if (ram_heap_size >= 0xC000) {
    ERROR("mm_init", "Ram heap size too large");
    return false;
  }

  ram_heap_end  = ram_heap_size;

  #if STATISTICS
    used_cells_count = 0;
    free_cells_count = 0;
    vector_cells_count = 0;
  #endif

  for (cell_p i = 0; i < reserved_cells_count; i++) {
    ram_heap[i].type = CONS_TYPE;
    ram_heap[i].cons.car_p = NIL;
    ram_heap[i].cons.cdr_p = NIL;
  }

  mm_sweep();

  if (!check_free_list(ram_heap_size)) return false;

  return true;
}


#if TESTS
void mm_tests()
{
  TESTM("mm");

  TEST("mm Initialisation");

    mm_init(25); // 13 cells required...
    EXPECT_TRUE(sizeof(cell) == 5, "Size of a single cell not equal to 5");
    EXPECT_TRUE((((char *) &vector_heap[0]) + sizeof(cell)) == ((char *) &vector_heap[1]), "Cell structure alignment problems");
    EXPECT_TRUE(check_free_list(ram_heap_size), "Check Ram Heap Free Size return wrong count");
    EXPECT_TRUE(global_count == 25, "Globals count != 25");
    EXPECT_TRUE(reserved_cells_count == 13, "Reserved cells count != 13");
    EXPECT_TRUE(ram_heap != NULL, "Ram Heap pointer is NULL");
    EXPECT_TRUE(vector_heap != NULL, "Vector Heap pointer is NULL");
    EXPECT_TRUE(ram_heap_size == RAM_HEAP_ALLOCATED, "Ram Heap Size is wrong");
    EXPECT_TRUE(vector_heap_size == VECTOR_HEAP_ALLOCATED, "Vector Heap Size is wrong");
    EXPECT_TRUE(vector_free_cells == 0, "Vector Free Cells pointer is wrong");
    EXPECT_TRUE(ram_heap_end == ram_heap_size, "Ram Heap End and Size not equal");
    EXPECT_TRUE(free_cells == 13, "Free Cells pointer must be pointing at index 0");
    EXPECT_TRUE((reg1 == NIL) && (reg2 == NIL) && (reg3 == NIL) && (reg4 == NIL) && (env == NIL) && (cont == NIL), "All registers not initialized to NIL");

  TEST("Heap allocations");

    cell_p p = mm_new_ram_cell();
    EXPECT_TRUE(p == 13, "Allocated Cell must be at index 13");
    EXPECT_TRUE(free_cells == 14, "Free Cells pointer expected to be at 14");

    for (int i = 0; i < 5000; i++) {
      RAM_SET_TYPE(p, CONS_TYPE);
      RAM_SET_CDR(p, env);
      RAM_SET_CAR(p, FALSE);
      env = p;
      p = mm_new_ram_cell();
    }

    EXPECT_TRUE(free_cells == 5014, "Free Cells pointer expected to be at 5014");

    RAM_SET_CAR(1000, 1005);
    mm_gc();
    EXPECT_TRUE(
      free_cells_count == (ram_heap_size - 13 - 5000),
      "Free Cells Count expected to be at %d but is %d",
      (ram_heap_size - 13 - 5000),
      free_cells_count
    );

    RAM_SET_CDR(2000, RAM_GET_CDR(2000) - 1000);
    mm_gc();
    EXPECT_TRUE(
      free_cells_count == (ram_heap_size - 13 - 4000),
      "Free Cells count expected to be at %d but is %d",
      (ram_heap_size - 13 - 4000),
      free_cells_count
    );

    env = NIL;
    mm_gc();
    EXPECT_TRUE(free_cells_count == (ram_heap_size - 13),
      "Free Cells count expected to be at %d but is %d",
      ram_heap_size - 13,
      free_cells_count
    );

  TEST("Vector allocations");

    vector_p v = mm_new_vector_cell(77, p);
    EXPECT_TRUE(v == 1, "Vector not pointing at first byte");
    EXPECT_TRUE(vector_free_cells == 17, "Vector Free Cells not pointing at index 17");
    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v - 1) == p, "Vector heap not pointing at vector header");
    EXPECT_TRUE(VECTOR_GET_LENGTH(v - 1) == 17, "Vector length in cells is wrong");
    EXPECT_TRUE(VECTOR_IS_USED(v - 1), "Vector is not marked as used");
    VECTOR_SET_BYTE(v, 35, 3);
    EXPECT_TRUE(VECTOR_GET_BYTE(v, 35) == 3, "Vector set/get not working");

  TEST("Vector compaction");

    cell_p p2, p3;
    vector_p v2, v3;
    p2 = mm_new_ram_cell();
    p3 = mm_new_ram_cell();

    v2 = mm_new_vector_cell(30, p2);
    RAM_SET_VECTOR_START(p2, v2);

    v3 = mm_new_vector_cell(140, p3);
    RAM_SET_VECTOR_START(p3, v3);
    RAM_SET_VECTOR_START(p, v);

    VECTOR_SET_BYTE(v3, 3, 7);
    VECTOR_SET_BYTE(v3, 131, 13);

    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v2 - 1) == p2, "p2 vector not set properly");
    EXPECT_TRUE(VECTOR_GET_RAM_PTR(v3 - 1) == p3, "p3 vector not set properly");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p2) == v2, "p2 not pointing at v2");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p3) == v3, "p3 not pointing at v3");
    EXPECT_TRUE(VECTOR_IS_USED(v2 - 1), "Vector v2 is not marked as used");
    EXPECT_TRUE(VECTOR_IS_USED(v3 - 1), "Vector v3 is not marked as used");

    VECTOR_SET_FREE(v2 - 1);
    EXPECT_TRUE(VECTOR_IS_FREE(v2 - 1), "Vector v2 is not free");
    mm_compact_vector_space();

    EXPECT_TRUE(RAM_GET_VECTOR_START(p2) == RAM_GET_VECTOR_START(p3), "Vector p3 not properly realigned after compaction");
    EXPECT_TRUE(RAM_GET_VECTOR_START(p) == 1, "Vector p start byte address not set properly");
    EXPECT_TRUE(VECTOR_GET_BYTE(RAM_GET_VECTOR_START(p3), 3) == 7, "Vector reallocation not complete (index 3 is wrong)");
    EXPECT_TRUE(VECTOR_GET_BYTE(RAM_GET_VECTOR_START(p3), 131) == 13, "Vector reallocation not complete (index 131 is wrong)");

    VECTOR_SET_FREE(v - 1);
    mm_compact_vector_space();
    EXPECT_TRUE(RAM_GET_VECTOR_START(p3) == 1, "Vector p3 not properly realigned after other vectors freed and compaction");

    VECTOR_SET_FREE(RAM_GET_VECTOR_START(p3) - 1);
    mm_compact_vector_space();

    EXPECT_TRUE(vector_free_cells == 0, "Vector Free Cells pointer is wrong");

}
#endif
