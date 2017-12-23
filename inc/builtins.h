#ifndef BUILTINS_H
#define BUILTINS_H

#ifdef CONFIG_DEBUG_STRINGS
  extern const char* const builtin_names[];
#endif /* CONFIG_DEBUG_STRINGS */

/* For the builtin scanning pass. */
#ifndef NO_BUILTIN_EXPAND

  #define BUILTIN(scheme_name, c_name, args, index) void builtin_ ## c_name ()
  #define BUILTIN_UNSPEC BUILTIN

#endif /* NO_BUILTIN_EXPAND */

#endif
