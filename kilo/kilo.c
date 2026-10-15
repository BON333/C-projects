/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/
struct termios orig_termios;  // estado original do terminal, para repor à saída

/*** terminal ***/
void die(const char *s) 
{
  perror(s);  // mostra a mensagem de erro do sistema (errno)
  exit(1);
}

void disableRawMode(void) 
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode(void) 
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);  // garante que o terminal volta ao normal

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);  // sem Ctrl-S/Q, sem \r->\n
  raw.c_oflag &= ~(OPOST);                                    // sem \n->\r\n automático
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);  // sem eco, tecla a tecla, sem Ctrl-C/Z/V
  raw.c_cc[VMIN] = 0;   // read() retorna mesmo sem bytes...
  raw.c_cc[VTIME] = 1;  // ...ao fim de 100 ms

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** init ***/
int main(void) 
{
  enableRawMode();

  while (1)
  {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);            // teclas de controlo: só o código
    } else {
      printf("%d ('%c')\r\n", c, c);  // teclas normais: código e caractere
    }
    if (c == 'q') break;
  }
  return 0;
}
