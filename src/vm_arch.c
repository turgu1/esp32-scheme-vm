#include "esp32_scheme_vm.h"

#define VM_ARCH
#include "vm_arch.h"

void init_vm_arch()
{
  root = NIL;
}