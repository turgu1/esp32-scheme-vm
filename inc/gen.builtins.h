#include "esp32-scheme-vm.h"
#include "builtins.h"

#ifdef CONFIG_DEBUG_STRINGS
const char * const builtin_names[] = {
  "#%halt",
  "return",
  "pop",
  "get-cont",
  "graft-to-cont",
  "return-to-cont",
  "pair?",
  "cons",
  "car",
  "cdr",
  "set-car!",
  "set-cdr!",
  "null?",
  "number?",
  "=",
  "#%+",
  "#%-",
  "#%mul-non-neg",
  "#%div-non-neg",
  "#%rem-non-neg",
  "<",
  ">",
  "bitwise-ior",
  "bitwise-xor",
  "bitwise-xor",
  "bitwise-xor",
  "eq?",
  "not",
  "symbol?",
  "boolean?",
  "string?",
  "string->list",
  "list->string",
  "u8vector?",
  "#%make-u8vector",
  "u8vector-ref",
  "u8vector-set!",
  "u8vector-length",
  "print",
  "clock",
  "#%getchar-wait",
  "#%putchar"
};
#endif /* CONFIG_DEBUG_STRINGS */

extern void builtin_return();
extern void builtin_pop();
extern void builtin_get_cont();
extern void builtin_graft_to_cont();
extern void builtin_return_to_cont();
extern void builtin_pair_p();
extern void builtin_cons();
extern void builtin_car();
extern void builtin_cdr();
extern void builtin_set_car_bang();
extern void builtin_set_cdr_bang();
extern void builtin_null_p();
extern void builtin_number_p();
extern void builtin_equal();
extern void builtin_add();
extern void builtin_sub();
extern void builtin_mul_non_neg();
extern void builtin_div_non_neg();
extern void builtin_rem_non_neg();
extern void builtin_lt();
extern void builtin_gt();
extern void builtin_bitwise_ior();
extern void builtin_bitwise_xor();
extern void builtin_bitwise_and();
extern void builtin_bitwise_not();
extern void builtin_eq_p();
extern void builtin_not();
extern void builtin_symbol_p();
extern void builtin_boolean_p();
extern void builtin_string_p();
extern void builtin_string2list();
extern void builtin_list2string();
extern void builtin_u8vector_p();
extern void builtin_make_u8vector();
extern void builtin_u8vector_ref();
extern void builtin_u8vector_set();
extern void builtin_u8vector_length();
extern void builtin_print();
extern void builtin_clock();
extern void builtin_getchar_wait();
extern void builtin_putchar();
