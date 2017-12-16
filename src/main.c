/* ESP32 Scheme VM

   This program is in the Public Domain

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Guy Turcotte
   December 2017
*/

#include "esp32-scheme-vm.h"
#include "vm-arch.h"
#include "mm.h"

bool initialisations()
{
  if (!mm_init()) return false;
  init_vm_arch();

  return true;
}

#ifdef COMPUTER
  int main()
#else
  void app_main()
#endif
{
  if (!initialisations()) {
    printf("ERROR - Unable to properly initialise the environment\n");
    #ifdef ESP32
      esp_restart();
      for (int i = 10; i >= 0; i--) {
          printf("Restarting in %d seconds...\n", i);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    #endif
  }

  printf("Cell size: %lu\n", sizeof(cell));
  printf("Execution completed\n");
  return 0;
}

#if 0
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "esp_system.h"
  #include "esp_spi_flash.h"
  #include "esp_heap_caps.h"

char * msg = "allo";

void app_main()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    heap_caps_print_heap_info(MALLOC_CAP_8BIT);

    long max_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    printf("---> Max block size: %ld\n", max_block);
    printf("Allo address: %08X\n", (unsigned int) msg);
    printf("Main address: %08X\n", (unsigned int) app_main);

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
#endif
