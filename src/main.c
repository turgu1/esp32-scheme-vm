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
#include "hexfile.h"
#include "kb.h"

bool testing = false;

#if TESTING
  #include "tests.h"

  void conduct_tests()
  {
    tested_count = failed_count = 0;

    kb_tests();
    mm_tests();
    vm_arch_tests();
    builtins_computer_tests();
    builtins_control_tests();
    builtins_list_tests();
    builtins_numeric_tests();
    builtins_util_tests();
    builtins_vector_tests();
    hexfile_tests();
    interpreter_tests();

    printf("\n\n--------------------\nTests completed: %d\nTests failed: %d\n--------------------\n", tested_count, failed_count);
  }
#endif

bool initialisations(char * program_filename)
{
  if ((program = calloc(65536, 1)) == NULL) {
    FATAL("initialisations", "Unable to allocate program space");
  }

  if (!read_hex_file(program_filename, program, 65536)) return false;

  if (!mm_init(*(program + 3))) return false;

  vm_arch_init();

  kb_init();

  return true;
}

void terminate()
{
  fflush(stdout);
  kb_restore();
  exit(1);
}

void usage(char * exename)
{
  fprintf(stderr, "Usage: %s [-t | -T] filename\n", exename);
}

#ifdef COMPUTER
  int main(int argc, char **argv)
#else
  void app_main()
#endif
{
  #ifdef COMPUTER
    int opt = 0;
    trace = false;
    char *fname;
    while ((opt = getopt(argc, argv, "tT")) != -1) {
      switch (opt) {
        #if TESTING
          case 'T':
            testing = true;
            conduct_tests();
            return 0;
            break;
        #endif
        #if TRACING
          case 't':
            trace = true;
            break;
          case '?':
            usage(argv[0]);
            return 1;
            break;
        #endif
      }
    }
    if ((optind > 0) && (argc > optind)) {
      fname = argv[optind];
    }
    else {
      usage(argv[0]);
      return 1;
    }
    if (!initialisations(fname)) {
  #else
    if (!initialisations("program.hex")) {
  #endif
    ERROR("main", "Unable to properly initialise the environment");
    #ifdef ESP32
      esp_restart();
      for (int i = 10; i >= 0; i--) {
          printf("Restarting in %d seconds...\n", i);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    #endif
  }

  INFO_MSG("main: Cell size: %lu\n", sizeof(cell));
  INFO("main", "Execution completed");

  terminate();

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
