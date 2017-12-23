// builtins-utils
// Builtin Indexes: 26..32

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

#include "builtins.h"

BUILTIN(eq?, eq_p, 2, 26)
{
	reg1 = ENCODE_BOOL(reg1 == reg2);
	reg2 = NIL;
}

BUILTIN(not, not, 1, 27)
{
	reg1 = ENCODE_BOOL(reg1 == NIL);
}

BUILTIN(symbol?, symbol_p, 1, 28)
{
	if (IN_RAM(reg1)) {
		reg1 = ENCODE_BOOL(RAM_IS_SYMBOL(reg1));
	}
  else if (IN_ROM(reg1)) {
		reg1 = ENCODE_BOOL(ROM_IS_SYMBOL(reg1));
	}
  else {
		reg1 = NIL;
	}
}

BUILTIN(boolean?, boolean_p, 1, 29)
{
	reg1 = ENCODE_BOOL((reg1 == FALSE) || (reg1 == TRUE));
}

BUILTIN(string?, string_p, 1, 30)
{
	if (IN_RAM(reg1)) {
		reg1 = ENCODE_BOOL(RAM_IS_STRING(reg1));
	}
  else if (IN_ROM(reg1)) {
		reg1 = ENCODE_BOOL(ROM_IS_STRING(reg1));
	}
  else {
		reg1 = NIL;
	}
}

BUILTIN(string->list, string2list, 1, 31)
{
	if (IN_RAM(reg1)) {
    EXPECT(RAM_IS_STRING(reg1), "string->list.0", "string");

		reg1 = RAM_GET_CAR(reg1);
	}
  else if (IN_ROM(reg1)) {
    EXPECT(ROM_IS_STRING(reg1), "string->list.1", "string");

		reg1 = ROM_GET_CAR(reg1);
	}
  else {
		TYPE_ERROR("string->list.2", "string");
	}
}

BUILTIN(list->string, list2string, 1, 32)
{
	reg1 = new_string(reg1);
}


#if TESTING
void builtins_util_tests()
{

}
#endif
