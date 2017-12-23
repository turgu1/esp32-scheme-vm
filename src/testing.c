#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

#if TESTS

#define TESTING 1
#include "testing.h"
#include "tests.h"

#include <stdarg.h>

static int failed_count;
static int tested_count;

void EXPECT_TRUE(bool val, char * info, ...)
{
  va_list ap;
  static char buf[200];


  tested_count++;
  putchar('.');
  fflush(stdout);
  if (!(val)) {
    va_start(ap, info);
    strcpy(buf, "\nTest failed: ");
    strncat(buf, info, 200 - 15);
    vprintf(buf, ap);
    putchar('\n');
    failed_count++;
  }
}

void conduct_tests()
{
  tested_count = failed_count = 0;

  //kb_tests(); // Requires user interaction
  mm_tests();
  vm_arch_tests();
  builtins_computer_tests();
  builtins_control_tests();
  builtins_list_tests();
  builtins_numeric_tests();
  builtins_util_tests();
  builtins_vector_tests();
  hexfile_tests();
  interpreter_tests();

  printf(
    "\n\n--------------------\nTests completed: %d\nTests failed: %d\n--------------------\n",
    tested_count,
    failed_count);
}

#endif
