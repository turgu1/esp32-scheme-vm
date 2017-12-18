#ifndef HEXFILE_H
#define HEXFILE_H

#ifdef HEXFILE
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

bool read_hex_file(char * filename, char * buffer, int size);

#undef PUBLIC
#endif
