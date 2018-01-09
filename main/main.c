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
#include "interpreter.h"
#include "testing.h"

bool initialisations(char * program_filename)
{
  if ((program = calloc(65536, 1)) == NULL) {
    FATAL("initialisations", "Unable to allocate program space");
  }

  if (!read_hex_file(program_filename, program, 65536)) return false;

  if (!mm_init(program)) return false;

  vm_arch_init();

  kb_init();

  return true;
}

void terminate()
{
  #if STATISTICS
    INFO_MSG("terminate: GC Processing Count: %d.", gc_call_counter);
    #if COMPUTER
      INFO_MSG("terminate: Max GC Duration: %10.7f Sec.", max_gc_duration);
    #endif
    fputc('\n', stderr);
  #endif

  fflush(stderr);
  fflush(stdout);
  kb_restore();
  exit(1);
}

void usage(char * exename)
{
  fprintf(stderr,
    "Usage: %s [options] filename\n"
    "\nOptions:\n"
    #if TRACING
      "  -t  Trace\n"
    #endif
    #if TESTS
      "  -T  Tests\n"
    #endif
    "  -v  Verbose\n"
    "  -V  Version\n"
    "  -?  Print this message\n", exename);
}

char * options = "vV?"
#if TRACING
  "t"
#endif
#if TESTS
  "T"
#endif
;

#ifdef COMPUTER
  int main(int argc, char **argv)
#else
  void app_main()
#endif
{
  #ifdef COMPUTER
    int opt = 0;
    trace = false;
    verbose = false;
    char *fname;
    while ((opt = getopt(argc, argv, options)) != -1) {
      switch (opt) {
        case 'v':
          verbose = true;
          break;
        #if TESTS
          case 'T':
            conduct_tests();
            return 0;
            break;
        #endif
        #if TRACING
          case 't':
            trace = true;
            break;
        #endif
        case '?':
          usage(argv[0]);
          return 1;
          break;
        case 'V':
          printf("%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
          return 0;
        default:
          printf("Unknown option!\n");
          usage(argv[0]);
          return 1;
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

  interpreter();

  #if DEBUGGING
    INFO("main", "Execution completed");
  #endif

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
