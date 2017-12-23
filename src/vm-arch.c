#include "esp32-scheme-vm.h"

#define VM_ARCH
#include "vm-arch.h"

#include "mm.h"

void vm_arch_init()
{
}

cell_p pop()
{
  // all the cons linking stack elements together are from the RAM Heap.
  #if DEBUGGING
    if ((env != NIL) && (ram_heap[env].type != CONS_TYPE)) {
      FATAL("pop.0", "HEAP BROKEN!!!");
    }
  #endif

  if (env == NIL) {
    #if DEBUGGING
      WARNING("pop.1", "Environment exhausted");
    #endif

    return NIL;
  }
  cell_p p = RAM_GET_CAR_NO_TEST(env);
  env = RAM_GET_CDR_NO_TEST(env);

  return p;
}

cell_p new_closure(cell_p env, code_p code)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = CLOSURE_TYPE;
  ram_heap[p].closure.environment_p = env;
  ram_heap[p].closure.entry_point_p  = code;

  return p;
}

cell_p new_cont(cell_p parent, cell_p closure)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = CONTINUATION_TYPE;
  ram_heap[p].continuation.closure_p = closure;
  ram_heap[p].continuation.parent_p  = parent;

  return p;
}

cell_p new_pair(cell_p car, cell_p cdr)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = CONS_TYPE;
  ram_heap[p].cons.car_p = car;
  ram_heap[p].cons.cdr_p = cdr;

  return p;
}

cell_p new_string(cell_p chars_p)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = STRING_TYPE;
  ram_heap[p].string.chars_p = chars_p;
  ram_heap[p].string.unused = 0;

  return p;
}

cell_p new_fixnum(int32_t value)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = FIXNUM_TYPE;
  ram_heap[p].fixnum.value = value;

  return p;
}

cell_p new_vector(uint16_t length)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type           = VECTOR_TYPE;
  ram_heap[p].vector.start_p = mm_new_vector_cell(length, p);
  ram_heap[p].vector.length  = length;

  return p;
}

int32_t decode_int(cell_p p)
{
  int val;

  if (p >= SMALL_INT_START) {
    if ((p == NIL) || (p == FALSE)) {
      val = 0;
    }
    else if (p == TRUE) {
      val = 1;
    }
    else {
      val = p & SMALL_INT_MASK;
      val--;
    }
  }
  else if (IN_RAM(p)) {
    EXPECT(RAM_IS_FIXNUM(p), "decode_int.0", "fixnum");

    val = RAM_GET_FIXNUM_VALUE(p);
  }
  else if (IN_ROM(p)) {
    EXPECT(ROM_IS_FIXNUM(p), "decode_int.1", "fixnum");

    val = ROM_GET_FIXNUM_VALUE(p);
  }
  else {
    TYPE_ERROR("decode_int.2", "fixnum");

    val = 0;
  }

  return val;
}

void decode_2_int_args ()
{
	a1 = decode_int(reg1);
	a2 = decode_int(reg2);
}

cell_p encode_int(int32_t val)
{
  if ((val >= -1) && (val <= 255)) {
    return (cell_p) (val + 1) | SMALL_INT_START;
  }
  else {
    return new_fixnum(val);
  }
}


#if TESTING
void vm_arch_tests()
{

}
#endif
