
#include "esp32-scheme-vm.h"
#include "testing.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

PRIVATE struct termios oldattr, newattr;
PRIVATE int oldf;

void kb_init()
{
  return;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
}

void kb_restore()
{
  return;
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
}

char kb_getch(void)
{
  int ch;

  ch = getchar();

  return (char)((ch == EOF) ? 0 : ch);
}

#if TESTS
void kb_tests()
{
  char ch = 0;

  TESTM("kb");

  TEST("Keyboard entry");

  kb_init();

  printf("Please press any key, followed with character 'X': ");
  while (ch != 'X') {
    ch = kb_getch();
    if (ch) printf("\nch = %c", ch);
  }
  putchar('\n');

  EXPECT_TRUE(ch == 'X', "kb_getch");

  kb_restore();
}
#endif
