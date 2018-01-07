#ifndef ESP32_SCHEME_VM_H
#define ESP32_SCHEME_VM_H

#define VERSION_MAJOR  1
#define VERSION_MINOR  0

extern void terminate();

#define COMPUTER   1
//#define ESP32 0

#ifdef ESP32
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "esp_system.h"
  #include "esp_spi_flash.h"
  #include "esp_heap_caps.h"
#endif

#ifdef COMPUTER
  #include <stdint.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <unistd.h>
  #include <string.h>

  typedef enum { false, true } bool;
#endif

#define STATS     0
#define DEBUGGING 1
#define TRACING   1
#define TESTS     1

#define CONFIG_BIGNUM_LONG 1

#define STATISTICS (STATS || DEBUGGING)

#if DEBUGGING
  #define DEBUG(format, ...) printf(format, ## __VA_ARGS__)
#else
  #define DEBUG(format, ...)
#endif

#if DEBUGGING
  #define   FATAL_MSG(format, ...) { printf(  "\nFATAL - In " format, ## __VA_ARGS__); putchar('\n'); fflush(stdout); terminate(); }
  #define   ERROR_MSG(format, ...) { printf(  "\nERROR - In " format, ## __VA_ARGS__); putchar('\n'); }
  #define WARNING_MSG(format, ...) { printf("\nWARNING - In " format, ## __VA_ARGS__); putchar('\n'); }
  #define    INFO_MSG(format, ...) { if (verbose) { printf(   "\nINFO - In " format, ## __VA_ARGS__); putchar('\n'); } }

  #define   FATAL(a, b) { printf(  "\nFATAL - In %s: %s.\n", a, b); fflush(stdout); terminate(); }
  #define   ERROR(a, b)   printf(  "\nERROR - In %s: %s.\n", a, b)
  #define WARNING(a, b)   printf("\nWARNING - In %s: %s.\n", a, b)
  #define    INFO(a, b)   if (verbose) { printf("\nINFO - In %s: %s.\n", a, b); }

  #define TYPE_ERROR(proc, exp) FATAL(proc, "Expecting \"" exp "\"")
  #define EXPECT(test, proc, exp) { if (!(test)) { printf("\nAt [%ld]: ", last_pc.c - program); TYPE_ERROR(proc, exp); } }
  #define MARK(c) { if (verbose) { putchar(c); fflush(stdout); } }
#else
  #define   FATAL_MSG(format, ...) terminate()
  #define   ERROR_MSG(format, ...)
  #define WARNING_MSG(format, ...)
  #define    INFO_MSG(format, ...)

  #define   FATAL(a, b) terminate()
  #define   ERROR(a, b)
  #define WARNING(a, b)
  #define    INFO(a, b)

  #define TYPE_ERROR(proc, exp)
  #define EXPECT(test, proc, exp)
  #define MARK(c)
#endif

#pragma pack(1)

#define PRIVATE static

bool trace;
bool verbose;

#endif
