#ifndef HEXFILE_H
#define HEXFILE_H

#ifdef HEXFILE
  #define PUBLIC
#else
  #define PUBLIC extern
#endif

bool read_hex_file(char * filename, uint8_t * buffer, int size);

#undef PUBLIC
#endif
