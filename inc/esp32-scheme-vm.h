#ifndef ESP32_SCHEME_VM_H
#define ESP32_SCHEME_VM_H

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

  typedef enum { false, true } bool;
#endif

#define STATS     1
#define DEBUGGING 1
#define TRACING   1

#define STATISTICS (STATS || DEBUGGING)

#if DEBUGGING
  #define DEBUG(format, ...) printf(format, ## __VA_ARGS__)
#else
  #define DEBUG(format, ...)
#endif

#if TRACING
  #define TRACE(format, ...) printf(format, ## __VA_ARGS__)
#else
  #define TRACE(format, ...)
#endif

#if DEBUGGING
  #define   FATAL(format, ...) printf("FATAL - "   # format, ## __VA_ARGS__)
  #define   ERROR(format, ...) printf("ERROR - "   # format, ## __VA_ARGS__)
  #define WARNING(format, ...) printf("WARNING - " # format, ## __VA_ARGS__)
  #define    INFO(format, ...) printf("INFO - "    # format, ## __VA_ARGS__)
#else
  #define   FATAL(format, ...)
  #define   ERROR(format, ...)
  #define WARNING(format, ...)
  #define    INFO(format, ...)
#endif

#pragma pack(1)

#define PRIVATE static

#endif
