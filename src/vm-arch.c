#include "esp32-scheme-vm.h"

#define VM_ARCH
#include "vm-arch.h"

#include "mm.h"
#include "testing.h"

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

cell_p new_pair(cell_p car, cell_p cdr)
{
  cell_p p = mm_new_ram_cell();

  ram_heap[p].type = CONS_TYPE;
  ram_heap[p].cons.car_p = car;
  ram_heap[p].cons.cdr_p = cdr;

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


#if TESTS
void vm_arch_tests()
{
  int i;
  cell_p p, q;

  vm_arch_init();

  TESTM("vm-arch");

  TEST("new_pair()");

    p = new_pair(NIL, FALSE);
    EXPECT_TRUE(RAM_IS_PAIR(p), "new_pair() does't not return a cons");
    EXPECT_TRUE(RAM_GET_CAR(p) == NIL, "CAR not properly set");
    EXPECT_TRUE(RAM_GET_CDR(p) == FALSE, "CDR not properly set");

  TEST("pop()");

    env = NIL;
    // A warning message will appear on console if debugging enabled
    EXPECT_TRUE(pop() == NIL, "Empty environment doesn't return NIL on pop()");

    env = new_pair(encode_int(1), encode_int(2));
    env = new_pair(p, env);

    q = pop();
    EXPECT_TRUE(RAM_GET_CAR(q) == NIL && RAM_GET_CDR(q) == FALSE, "pop() doesn't return the top env cell");

  TEST("new_closure()");


  TEST("new_cont()");


  TEST("new_fixnum()");


  TEST("new_bignum()");
  // Not yet

  TEST("new_string()");


  TEST("new_cstring()");
  // Not yet

  TEST("new_vector()");


  TEST("encode_int()");

    for (i = -1; i < 256; i++) {
      EXPECT_TRUE(encode_int(i) == (SMALL_INT_START + i + 1), "Small Int encoding wrong");
    }
    p = encode_int(1000);
    EXPECT_TRUE(RAM_IS_FIXNUM(p), "Integer encoding does not return a FIXNUM");
    EXPECT_TRUE(RAM_GET_FIXNUM_VALUE(p) == 1000, "Expected encoded value is not 1000");

  TEST("decode_int()");

    for (q = SMALL_INT_START, i = -1; q <= SMALL_INT_MAX; q++, i++) {
      EXPECT_TRUE(decode_int(q) == i, "Small Int decoding wrong");
    }
    EXPECT_TRUE(decode_int(p) == 1000, "Expected decoded value is not 1000");
}
#endif
