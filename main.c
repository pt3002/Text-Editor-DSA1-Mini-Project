#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/**define**/
#define CTRL_KEY(k) ((k)& 0x1f)


struct termios orig_termios;

/**terminal**/
void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode() { 
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)==-1)
        die("tcsetattr");
}

void enableRawMode() 
{
    // tcgetattr(STDIN_FILENO, &orig_termios);
    if(tcgetattr(STDIN_FILENO, &orig_termios)==-1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(ICRNL| INPCK |ISTRIP| IXON | BRKINT);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |=(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1)
        die ("tcsetattr");
 }

 /**init**/

int main() {
  enableRawMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != CTRL_KEY('q')) {
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  }
  return 0;
}
