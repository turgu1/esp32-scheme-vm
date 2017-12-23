// builtins-numeric
// Builtin Indexes: 13..25

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

#include "builtins.h"

BUILTIN(number?, number_p, 1, 13)
{
	if ((reg1 >= SMALL_INT_START) && (reg1 <= SMALL_INT_MAX)) {
		reg1 = TRUE;
	}
  else {
		if (IN_RAM(reg1)) {
			reg1 = ENCODE_BOOL(RAM_IS_FIXNUM(reg1) || RAM_IS_BIGNUM(reg1));
		}
    else if (IN_ROM(reg1)) {
			reg1 = ENCODE_BOOL(ROM_IS_FIXNUM(reg1) || ROM_IS_BIGNUM(reg1));
		}
    else {
			reg1 = FALSE;
		}
	}
}

BUILTIN(=, equal, 2, 14)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = ENCODE_BOOL(cmp(reg1, reg2) == 1);
#else
	decode_2_int_args();
	reg1 = ENCODE_BOOL(a1 == a2);
#endif
	reg2 = FALSE;
}

BUILTIN(#%+, add, 2, 15)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = add (reg1, reg2);
#else
	decode_2_int_args();
	reg1 = encode_int(a1 + a2);
#endif
	reg2 = NIL;
}

BUILTIN(#%-, sub, 2, 16)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = sub (reg1, reg2);
#else
	decode_2_int_args();
	reg1 = encode_int(a1 - a2);
#endif
	reg2 = NIL;
}

BUILTIN(#%mul-non-neg, mul_non_neg, 2, 17)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = mulnonneg (reg1, reg2);
#else
	decode_2_int_args();
	reg1 = encode_int(a1 * a2);
#endif
	reg2 = NIL;
}

BUILTIN(#%div-non-neg, div_non_neg, 2, 18)
{
#ifdef CONFIG_BIGNUM_LONG
	if (obj_eq(reg2, ZERO)) {
		ERROR("quotient", "divide by 0");
	}

	reg1 = divnonneg (reg1, reg2);
#else
	decode_2_int_args ();

	if (a2 == 0) {
		ERROR("quotient", "divide by 0");
	}

	reg1 = encode_int(a1 / a2);
#endif

	reg2 = NIL;
}

BUILTIN(#%rem-non-neg, rem_non_neg, 2, 19)
{
#ifdef CONFIG_BIGNUM_LONG
	if (obj_eq(reg2, ZERO)) {
		ERROR("remainder", "divide by 0");
	}

	reg3 = divnonneg (reg1, reg2);
	reg4 = mulnonneg (reg2, reg3);
	reg1 = sub(reg1, reg4);
	reg3 = NIL;
	reg4 = NIL;
#else
	decode_2_int_args ();

	if (a2 == 0) {
		ERROR("remainder", "divide by 0");
	}

	reg1 = encode_int(a1 % a2);
#endif

	reg2 = NIL;
}

BUILTIN(<, lt, 2, 20)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = ENCODE_BOOL(cmp (reg1, reg2) < 1);
#else
	decode_2_int_args ();
	reg1 = ENCODE_BOOL(a1 < a2);
#endif
	reg2 = NIL;
}

BUILTIN(>, gt, 2, 21)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = ENCODE_BOOL(cmp (reg1, reg2) > 1);
#else
	decode_2_int_args ();
	reg1 = ENCODE_BOOL(a1 > a2);
#endif
	reg2 = NIL;
}

BUILTIN(bitwise-ior, bitwise_ior, 2, 22)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = bitwise_ior(reg1, reg2);
#else
	decode_2_int_args ();
	reg1 = encode_int(a1 | a2);
#endif
	reg2 = NIL;
}

BUILTIN(bitwise-xor, bitwise_xor, 2, 23)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = bitwise_xor(reg1, reg2);
#else
	decode_2_int_args ();
	reg1 = encode_int(a1 ^ a2);
#endif
	reg2 = NIL;
}

BUILTIN(bitwise-xor, bitwise_and, 2, 24)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = bitwise_xor(reg1, reg2);
#else
	decode_2_int_args ();
	reg1 = encode_int(a1 & a2);
#endif
	reg2 = NIL;
}

BUILTIN(bitwise-xor, bitwise_not, 1, 25)
{
#ifdef CONFIG_BIGNUM_LONG
	reg1 = bitwise_not(reg1);
#else
	reg1 = encode_int(~ decode_int(reg1));
#endif
}


#if TESTING
void builtins_numeric_tests()
{

}
#endif
