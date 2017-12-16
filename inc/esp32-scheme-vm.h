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

#define STATISTICS (STATS || DEBUGGING)

#pragma pack(1)

#define PRIVATE static

#endif
