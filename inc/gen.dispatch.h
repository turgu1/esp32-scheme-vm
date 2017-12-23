      case BUILTIN1 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "#%halt", 0);
            return;

          case 1 :
            TRACE("  (%s <%d>)\n", "return", 1);
            reg1 = pop ();
            builtin_return ();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "pop", 0);
            builtin_pop ();
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "get-cont", 0);
            builtin_get_cont ();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "graft-to-cont", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_graft_to_cont ();
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", "return-to-cont", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_return_to_cont ();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "pair?", 1);
            reg1 = pop ();
            builtin_pair_p ();
            env = new_pair(reg1, env);
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "cons", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_cons ();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "car", 1);
            reg1 = pop ();
            builtin_car ();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "cdr", 1);
            reg1 = pop ();
            builtin_cdr ();
            env = new_pair(reg1, env);
            break;

          case 10 :
            TRACE("  (%s <%d>)\n", "set-car!", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_set_car_bang ();
            break;

          case 11 :
            TRACE("  (%s <%d>)\n", "set-cdr!", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_set_cdr_bang ();
            break;

          case 12 :
            TRACE("  (%s <%d>)\n", "null?", 1);
            reg1 = pop ();
            builtin_null_p ();
            env = new_pair(reg1, env);
            break;

          case 13 :
            TRACE("  (%s <%d>)\n", "number?", 1);
            reg1 = pop ();
            builtin_number_p ();
            env = new_pair(reg1, env);
            break;

          case 14 :
            TRACE("  (%s <%d>)\n", "=", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_equal ();
            env = new_pair(reg1, env);
            break;

          case 15 :
            TRACE("  (%s <%d>)\n", "#%+", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_add ();
            env = new_pair(reg1, env);
            break;
        }
        break;

      case BUILTIN2 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "#%-", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_sub ();
            env = new_pair(reg1, env);
            break;

          case 1 :
            TRACE("  (%s <%d>)\n", "#%mul-non-neg", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_mul_non_neg ();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "#%div-non-neg", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_div_non_neg ();
            env = new_pair(reg1, env);
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "#%rem-non-neg", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_rem_non_neg ();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "<", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_lt ();
            env = new_pair(reg1, env);
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", ">", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_gt ();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "bitwise-ior", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_bitwise_ior ();
            env = new_pair(reg1, env);
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "bitwise-xor", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_bitwise_xor ();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "bitwise-xor", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_bitwise_and ();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "bitwise-xor", 1);
            reg1 = pop ();
            builtin_bitwise_not ();
            env = new_pair(reg1, env);
            break;

          case 10 :
            TRACE("  (%s <%d>)\n", "eq?", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_eq_p ();
            env = new_pair(reg1, env);
            break;

          case 11 :
            TRACE("  (%s <%d>)\n", "not", 1);
            reg1 = pop ();
            builtin_not ();
            env = new_pair(reg1, env);
            break;

          case 12 :
            TRACE("  (%s <%d>)\n", "symbol?", 1);
            reg1 = pop ();
            builtin_symbol_p ();
            env = new_pair(reg1, env);
            break;

          case 13 :
            TRACE("  (%s <%d>)\n", "boolean?", 1);
            reg1 = pop ();
            builtin_boolean_p ();
            env = new_pair(reg1, env);
            break;

          case 14 :
            TRACE("  (%s <%d>)\n", "string?", 1);
            reg1 = pop ();
            builtin_string_p ();
            env = new_pair(reg1, env);
            break;

          case 15 :
            TRACE("  (%s <%d>)\n", "string->list", 1);
            reg1 = pop ();
            builtin_string2list ();
            env = new_pair(reg1, env);
            break;
        }
        break;

      case BUILTIN3 :
        switch (instr & 0x0F) {
          case 0 :
            TRACE("  (%s <%d>)\n", "list->string", 1);
            reg1 = pop ();
            builtin_list2string ();
            env = new_pair(reg1, env);
            break;

          case 1 :
            TRACE("  (%s <%d>)\n", "u8vector?", 1);
            reg1 = pop ();
            builtin_u8vector_p ();
            env = new_pair(reg1, env);
            break;

          case 2 :
            TRACE("  (%s <%d>)\n", "#%make-u8vector", 1);
            reg1 = pop ();
            builtin_make_u8vector ();
            env = new_pair(reg1, env);
            break;

          case 3 :
            TRACE("  (%s <%d>)\n", "u8vector-ref", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_u8vector_ref ();
            env = new_pair(reg1, env);
            break;

          case 4 :
            TRACE("  (%s <%d>)\n", "u8vector-set!", 3);
            reg3 = pop ();
            reg2 = pop ();
            reg1 = pop ();
            builtin_u8vector_set ();
            break;

          case 5 :
            TRACE("  (%s <%d>)\n", "u8vector-length", 1);
            reg1 = pop ();
            builtin_u8vector_length ();
            env = new_pair(reg1, env);
            break;

          case 6 :
            TRACE("  (%s <%d>)\n", "print", 1);
            reg1 = pop ();
            builtin_print ();
            break;

          case 7 :
            TRACE("  (%s <%d>)\n", "clock", 0);
            builtin_clock ();
            env = new_pair(reg1, env);
            break;

          case 8 :
            TRACE("  (%s <%d>)\n", "#%getchar-wait", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_getchar_wait ();
            env = new_pair(reg1, env);
            break;

          case 9 :
            TRACE("  (%s <%d>)\n", "#%putchar", 2);
            reg2 = pop ();
            reg1 = pop ();
            builtin_putchar ();
            break;
        }
        break;

