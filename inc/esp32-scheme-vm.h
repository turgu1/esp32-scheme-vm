#ifndef ESP32_SCHEME_VM_H
#define ESP32_SCHEME_VM_H

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

#define STATS     1
#define DEBUGGING 1
#define TRACING   1
#define TESTING   1

#define STATISTICS (STATS || DEBUGGING)

#if TESTING
  #define TEST(name) printf("\n----- %s... -----\n", name)

  #define EXPECT_TRUE(val, info) { tested_count++; putchar('.'); fflush(stdout); if (!(val)) { printf("\nTest failed: %s\n", info); failed_count++; } }

  int tested_count;
  int failed_count;
#endif

#if DEBUGGING
  #define DEBUG(format, ...) printf(format, ## __VA_ARGS__)
#else
  #define DEBUG(format, ...)
#endif

#if TRACING
  #define TRACE(format, ...) { if (trace) printf(format, ## __VA_ARGS__); }
#else
  #define TRACE(format, ...)
#endif

#if DEBUGGING
  #define   FATAL_MSG(format, ...) { printf(  "\nFATAL - In " # format, ## __VA_ARGS__); terminate(); }
  #define   ERROR_MSG(format, ...)   printf(  "\nERROR - In " # format, ## __VA_ARGS__)
  #define WARNING_MSG(format, ...)   printf("\nWARNING - In " # format, ## __VA_ARGS__)
  #define    INFO_MSG(format, ...)   printf(   "\nINFO - In " # format, ## __VA_ARGS__)

  #define   FATAL(a, b) { printf(  "\nFATAL - In %s: %s.\n", a, b); terminate(); }
  #define   ERROR(a, b)   printf(  "\nERROR - In %s: %s.\n", a, b)
  #define WARNING(a, b)   printf("\nWARNING - In %s: %s.\n", a, b)
  #define    INFO(a, b)   printf(   "\nINFO - In %s: %s.\n", a, b)

  #define TYPE_ERROR(proc, exp) ERROR(proc, "Expecting " # exp)
  #define EXPECT(test, proc, exp) { if (!(test)) TYPE_ERROR(proc, exp); }
#else
  #define   FATAL_MSG(format, ...) terminate()
  #define   ERROR_MSG(format, ...)
  #define WARNING_MSG(format, ...)
  #define    INFO_MSG(format, ...)

  #define   FATAL_MSG(a, b) terminate()
  #define   ERROR_MSG(a, b)
  #define WARNING_MSG(a, b)
  #define    INFO_MSG(a, b)

  #define TYPE_ERROR(proc, exp)
  #define EXPECT(test, proc, exp)
#endif

#pragma pack(1)

#define PRIVATE static

bool trace;

#endif
