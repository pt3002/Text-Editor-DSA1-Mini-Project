//Libraries that need to be included
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Defining all structures and enums**/
//struct for storing a row 
typedef struct row
{
     int size;
     char *chars;
     struct row *prev;
     struct row *next;
     char *render;
     int rsize;
} row;

//struct for opening editor in terminal
struct termios orig_termios;

//struct which will contain height and width of the teminal
struct editorConfig
{
     int cx, cy;
     int screenrows;
     int screencols;
     int no_of_rows;
     int rowoff;
     row *row;
     struct termios orig_termios;
     char filename[1000];
     char statusmsg[80];
     time_t statusmsg_time;
} E;

//append buffer struct
struct abuf
{
     char *b;
     int len;
};

//linked list struct for storing all rows
struct ll
{
     struct ll *next;
     char data[1000];
};

//enum for keys
enum editorKey
{
     BACKSPACE = 127,
     ARROW_LEFT = 1000,
     ARROW_RIGHT,
     ARROW_UP,
     ARROW_DOWN,
     DEL_KEY
};

/**All structures and enums defined**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**All function definitions**/

void editorInsertRow(int at, char *s, size_t len);
struct ll *createll(struct ll *head, char A[1000]);
void welcome_message();
void die(const char *s);
void disableRawMode();
void enableRawMode();
int editorReadKey();
int getWindowSize(int *rows, int *cols);
void editorUpdateRow(row *row);
void editorRowDelChar(row *row, int at);
void editorDelChar();
void editorAppendRow(char *s, size_t len);
void editorRowInsertChar(row *row, int at, int c);
void editorInsertChar(int c);
void editorDrawStatusBar(struct abuf *ab);
void editorKeypress();
void initEditor();
void editorMoveCursor(int key);
void refreshScreen();
void editorRows(struct abuf *ab);
void abFree(struct abuf *ab);
void fileOpen(char filename[1000]);
void editorSave();
void saving_to_file(struct ll *head, char filename[1000]);
void editorOpen(struct ll *head);
void editorInsertNewline();
void editorSetStatusMessage(const char *fmt, ...);
char *editorPrompt(char *prompt);
void editorFind();
void editorScroll();
int editorRowRxToCx(row *row, int rx);
int add(int a, int b, int mod);
int mult(int a, int b, int mod);
int power(int a, int b, int mod);
void precal();
void build_hash(char *s);
int get_hash(int x, int y);
int final_hash(char *s);
void rabinKarp(char *text, char *pattern);
void changehead(char filename[1000]);
void abAppend(struct abuf *ab, const char *s, int len);

/**Function Definition ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Defining all constants**/

#define TEXT_EDITOR_VERSION "0.1"
#define CTRL_KEY(k) ((k)&0x1f)
#define TAB_STOP 8
#define ABUF_INIT \
  {               \
    NULL, 0       \
  }

/**Constant definition ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Defining all flags and counters**/

int dirty_flag = 0;      // 0==>Ready to write    100==>Pressed CTRL+S    //200==>CTRL+Q without saving
int quit_flag = 0;       // 0==>File Not saved yet      1==>file Saved
int help_flag = 1;       // 0==>CRTL+h not pressed, no help needed      1==>ctrl is press, Display Help
struct ll *temphead = NULL;
const int mod = 1e9 + 7;
const int base = 31;
const int N = 1e7;
int *pw = NULL;
int *inv = NULL;
int *has = NULL;
int rk = 0;              // 0-> not found    //1->found
int position = -1;       //-1->Not found on any position   //any other no. if it is found
int row_number = -1;     //-1 -> Not found on any row          //any other no. if found

/**Definition ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Starting Message function**/

void welcome_message()
{
     printf("\n");
     printf("\t\t\t\t\t\t\t  Welcome to TEXT EDITOR ! \n\n");
     printf("\t\t\t\t\t\t\t    Please see codes given below\n");
     printf("\n");
     printf("\t\t\t\t -------------------------------------------------------------------------------------- \n");
     printf("\t\t\t\t|                            |                            |                            |\n");
     printf("\t\t\t\t|");
     printf("    OPEN AN EXISTING FILE   ");
     printf("|");
     printf("     CREATE A NEW FILE      ");
     printf("|");
     printf("         TO EXIT            ");
     printf("|\n");
     printf("\t\t\t\t|                            |                            |                            |\n");
     printf("\t\t\t\t|--------------------------------------------------------------------------------------|\n");
     printf("\t\t\t\t|                            |                            |                            |\n");
     printf("\t\t\t\t|");
     printf("             1              ");
     printf("|");
     printf("             2              ");
     printf("|");
     printf("             3              ");
     printf("|\n");
     printf("\t\t\t\t|                            |                            |                            |\n");
     printf("\t\t\t\t -------------------------------------------------------------------------------------- \n");
     printf("\n\n");
}

/**Starting Message function ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Functions for setting up the terminal**/

void die(const char *s)
{
     write(STDOUT_FILENO, "\x1b[2J", 4);
     write(STDOUT_FILENO, "\x1b[H", 3);

     perror(s);
     exit(1);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
  {
       die("tcsetattr");
  }
}

void enableRawMode()
{
     if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
     {
          die("tcgetattr");
     }
     atexit(disableRawMode);

     struct termios raw = E.orig_termios;
     raw.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT);
     raw.c_oflag &= ~(OPOST);
     raw.c_cflag |= (CS8);
     raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
     if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
     {
          die("tcsetattr");
     }
}

int getWindowSize(int *rows, int *cols)
{
     struct winsize ws;

     if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
     {
          return -1;
     }
     // on failure it will return -1

     else
     {
          *cols = ws.ws_col;
          *rows = ws.ws_row;
     }
}

/**Functions for setting up the terminal ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Function to create linked list of all rows**/

struct ll *createll(struct ll *head, char A[1000])
{
     struct ll *first, *t, *last;
     first = head;
     last = first;
     if (first == NULL)
     {
          first = (struct ll *)malloc(sizeof(struct ll));
          first->next = NULL;
          strcpy(first->data, A);
          last = first;
     }
     else
     {
          while (last->next != NULL)
          {
               last = last->next;
          }
          t = (struct ll *)malloc(sizeof(struct ll));
          t->next = NULL;
          last->next = t;
          strcpy(t->data, A);
     }
     return first;
}

/**Linked list creation function ends***/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Function for opening a file**/

void fileOpen(char filename[1000])
{
     FILE *fptr;
     fptr = fopen(filename, "r");
     if (fptr == NULL)
     {
          exit(0);
     }
     char c = fgetc(fptr);
     int i = 0;
     char s[1000] = "";
     char s1[1000] = "";
     int n = 0;
     struct ll *head;
     head = NULL;
     while (c != EOF)
     {
          if (c == '\n')
          {
               c = fgetc(fptr);
               head = createll(head, s);
               strcpy(s, s1);
               n++;
          }
          else
          {
               strncat(s, &c, 1);
               c = fgetc(fptr);
          }
     }
     temphead = head;
     editorOpen(head);
     fclose(fptr);
}

//Function for changing head pointer when file contents change
void changehead(char filename[1000])
{
     FILE *fptr;
     fptr = fopen(filename, "r");
     if (fptr == NULL)
     {
          exit(0);
     }
     char c = fgetc(fptr);
     int i = 0;
     char s[1000] = "";
     char s1[1000] = "";
     int n = 0;
     struct ll *head;
     head = NULL;
     while (c != EOF)
     {
          if (c == '\n')
          {
               c = fgetc(fptr);
               head = createll(head, s);
               // editorOpen(s,strlen(s));
               strcpy(s, s1);
               n++;
          }
          else
          {
               strncat(s, &c, 1);
               c = fgetc(fptr);
          }
     }
     temphead = head;
     fclose(fptr);
}

/**Function ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Setting up the Editor**/
void editorOpen(struct ll *head)
{
     while (head != NULL)
     {
          editorAppendRow(head->data, strlen(head->data));
          head = head->next;
     }
}

//Editor Read Key Function
int editorReadKey()
{
     int r;
     char c;
     while ((r = read(STDIN_FILENO, &c, 1)) != 1)
     {
          if (r == -1 && errno != EAGAIN)
          {
               die("read");
          }
     }

     if (c == '\x1b')
     {
          char seq[3];
          if (read(STDIN_FILENO, &seq[0], 1) != 1)
          {
               return '\x1b';
          }
          if (read(STDIN_FILENO, &seq[1], 1) != 1)
          {
               return '\x1b';
          }
          if (seq[0] == '[')
          {
               switch (seq[1])
               {
                    case 'A':
                         return ARROW_UP;

                    case 'B':
                         return ARROW_DOWN;

                    case 'C':
                         return ARROW_RIGHT;
                         
                    case 'D':
                         return ARROW_LEFT;
               }
          }
          return '\x1b';
     }
     else
     {
          return c;
     }
}

//Key Press Function
void editorKeypress()
{
     int c = editorReadKey();
     switch (c)
     {
          case '\r':
               editorInsertNewline();
               break;

          case CTRL_KEY('f'):
               editorFind();
               break;

          case CTRL_KEY('h'):
               help_flag = 1;
               break;

          case CTRL_KEY('s'):
               editorSave();
               break;

          case CTRL_KEY('q'):
               write(STDOUT_FILENO, "\x1b[2J", 4);
               write(STDOUT_FILENO, "\x1b[H", 3);
               if (quit_flag == 1)
               {
                    exit(0);
               }
               else
               {
                    dirty_flag = 200;
               }
               break;

          case BACKSPACE:
               editorDelChar();
               break;

          case ARROW_UP:
          case ARROW_DOWN:
          case ARROW_LEFT:
          case ARROW_RIGHT:
               editorMoveCursor(c);
               break;

          default:
               editorInsertChar(c);
               break;
     }
}

//editor move cursor operation
void editorMoveCursor(int key)
{
     switch (key)
     {
          case ARROW_LEFT:
               if (E.cx != 0)
               {
                    E.cx--;
               }
               break;

          case ARROW_RIGHT:
               if (E.cx != E.screencols - 1)
               {
                    E.cx++;
               }
               break;

          case ARROW_UP:
               if (E.cy != 0)
               {
                    E.cy--;
               }
               break;

          case ARROW_DOWN:
               if (E.cy != E.screenrows - 1)
               {
                    E.cy++;
               }
               break;
     }
}

void editorScroll()
{
     if (E.cy < E.rowoff)
     {
          E.rowoff = E.cy;
     }
     if (E.cy >= E.rowoff + E.screenrows)
     {
          E.rowoff = E.cy - E.screenrows + 1;
     }
}

/**Setting up the Editor Completed**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

//Status Bar shows the number of lines written
/**Function for drawing status bar**/
void editorDrawStatusBar(struct abuf *ab)
{
     //Help status bar
     if (help_flag == 1)
     {
          abAppend(ab, "\x1b[7m", 4);
          char status[80];
          char rstatus[80];
          int len = snprintf(status, sizeof(status), "ctrl+S: SAVE   ctrl+Q: QUIT   ctrl+F: FIND");
          int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.no_of_rows);
          if (len > E.screencols)
          {
               len = E.screencols;
          }
          abAppend(ab, status, len);
          while (len < E.screencols)
          {
               if (E.screencols - len == rlen)
               {
                    abAppend(ab, rstatus, rlen);
                    break;
               }
               else
               {
                    abAppend(ab, " ", 1);
                    len++;
               }
          }
          abAppend(ab, "\x1b[m", 3);
          abAppend(ab, "\r\n", 2);
          help_flag = 0;
     }

     // Quiting without Saving the file(Warning)
     else if (quit_flag == 0 && dirty_flag == 200)
     {
          abAppend(ab, "\x1b[7m", 4);
          char status[80];
          char rstatus[80];
          int len = snprintf(status, sizeof(status), "Please save before quitting !!!");
          int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.no_of_rows);
          if (len > E.screencols)
          {
               len = E.screencols;
          }
          abAppend(ab, status, len);
          while (len < E.screencols)
          {
               if (E.screencols - len == rlen)
               {
                    abAppend(ab, rstatus, rlen);
                    break;
               }
               else
               {
                    abAppend(ab, " ", 1);
                    len++;
               }
          }
          abAppend(ab, "\x1b[m", 3);
          abAppend(ab, "\r\n", 2);
          dirty_flag = 0;
     }

     // Message for Saving
     if (dirty_flag == 100)
     {
          abAppend(ab, "\x1b[7m", 4);
          char status[80];
          char rstatus[80];
          int len = snprintf(status, sizeof(status), "Saved Succesfully");
          int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.no_of_rows);
          if (len > E.screencols)
          {
               len = E.screencols;
          }
          abAppend(ab, status, len);
          while (len < E.screencols)
          {
               if (E.screencols - len == rlen)
               {
                    abAppend(ab, rstatus, rlen);
                    break;
               }
               else
               {
                    abAppend(ab, " ", 1);
                    len++;
               }
          }
          abAppend(ab, "\x1b[m", 3);
          abAppend(ab, "\r\n", 2);
          dirty_flag = 0;
     }

     // Message for Normal writing
     else
     {
          abAppend(ab, "\x1b[7m", 4);
          char status[80];
          char rstatus[80];
          int len = snprintf(status, sizeof(status), "%.20s - %d lines",
                              E.filename ? E.filename : "[No Name]", E.no_of_rows);
          int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.no_of_rows);
          if (len > E.screencols)
          {
               len = E.screencols;
          }
          abAppend(ab, status, len);
          while (len < E.screencols)
          {
               if (E.screencols - len == rlen)
               {
                    abAppend(ab, rstatus, rlen);
                    break;
               }
               else
               {
                    abAppend(ab, " ", 1);
                    len++;
               }
          }
          abAppend(ab, "\x1b[m", 3);
          abAppend(ab, "\r\n", 2);
     }
}

/**Function for setting the message in status bar**/
void editorSetStatusMessage(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
     va_end(ap);
     E.statusmsg_time = time(NULL);
}

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Writting in the Text Editor**/

//Editor Rows function
void editorRows(struct abuf *ab)
{
     int y;
     for (y = 0; y < E.screenrows; y++)
     {
          int filerow = y + E.rowoff;
          if (filerow >= E.no_of_rows)
          {
               if (E.no_of_rows == 0 && y == E.screenrows / 3)
               {
                    char welcome[80];
                    int welcomelen = snprintf(welcome, sizeof(welcome), "Welcome To Text Editor %s", TEXT_EDITOR_VERSION);
                    if (welcomelen > E.screencols)
                    {
                         welcomelen = E.screencols;
                    }
                    int padding = (E.screencols - welcomelen) / 2;
                    if (padding)
                    {
                         abAppend(ab, "~", 1);
                         padding--;
                    }
                    while (padding--)
                    {
                         abAppend(ab, " ", 1);
                    }
                    abAppend(ab, welcome, welcomelen);
               }
               else
               {
                    abAppend(ab, "~", 1);
               }
          }
          else
          {
               int len = E.row[filerow].size;
               if (len > E.screencols)
               {
                    len = E.screencols;
               }
               abAppend(ab, E.row[filerow].chars, len);
          }
          abAppend(ab, "\x1b[K", 3);
          abAppend(ab, "\r\n", 2);
     }
}

void editorUpdateRow(row *row)
{
     free(row->render);
     row->render = malloc(row->size + 1);
     int j;
     int idx = 0;
     for (j = 0; j < row->size; j++)
     {
          row->render[idx++] = row->chars[j];
     }
     row->render[idx] = '\0';
     row->rsize = idx;
}

void editorInsertRow(int at, char *s, size_t len)
{
     if (at < 0 || at > E.no_of_rows)
          return;
     E.row = realloc(E.row, sizeof(row) * (E.no_of_rows + 1));
     memmove(&E.row[at + 1], &E.row[at], sizeof(row) * (E.no_of_rows - at));

     E.row[at].size = len;
     E.row[at].chars = malloc(len + 1);
     memcpy(E.row[at].chars, s, len);
     E.row[at].chars[len] = '\0';
     E.row[at].rsize = 0;
     E.row[at].render = NULL;
     editorUpdateRow(&E.row[at]);
     E.no_of_rows++;
     dirty_flag = 0;
}

void editorAppendRow(char *s, size_t len)
{
     E.row = realloc(E.row, sizeof(row) * (E.no_of_rows + 1));
     int at = E.no_of_rows;
     E.row[at].size = len;
     E.row[at].chars = malloc(len + 1);
     memcpy(E.row[at].chars, s, len);
     E.row[at].chars[len] = '\0';
     E.no_of_rows++;

     E.row[at].rsize = 0;
     E.row[at].render = NULL;

}

void editorInsertNewline()
{
     if (E.cx == 0)
     {
          editorInsertRow(E.cy, "", 0);
     }
     else
     {
          row *row = &E.row[E.cy];
          editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
          row = &E.row[E.cy];
          row->size = E.cx;
          row->chars[row->size] = '\0';
          editorUpdateRow(row);
     }
     E.cy++;
     E.cx = 0;
}

void editorInsertChar(int c)
{
     if (E.cy == E.no_of_rows)
     {
          editorInsertRow(E.no_of_rows, "", 0);
     }
     editorRowInsertChar(&E.row[E.cy], E.cx, c);
     E.cx++;
     quit_flag = 0;
}

void editorRowInsertChar(row *row, int at, int c)
{
     if (at < 0 || at > row->size)
     {
          at = row->size;
     }
     row->chars = realloc(row->chars, row->size + 2);
     memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
     row->size++;
     row->chars[at] = c;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//

//Deleting a character
void editorRowDelChar(row *row, int at)
{
     if (at < 0 || at >= row->size)
     {
          return;
     }
     memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
     row->size--;
     editorUpdateRow(row);
}

void editorDelChar()
{
     if (E.cy == E.no_of_rows)
     return;
     row *row = &E.row[E.cy];
     if (E.cx > 0)
     {
          editorRowDelChar(row, E.cx - 1);
          E.cx--;
     }
}

int editorRowRxToCx(row *erow, int rx)
{
     int cur_rx = 0;
     int cx;
     for (cx = 0; cx < erow->size; cx++)
     {
          if (erow->chars[cx] == '\t')
          {
               cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
          }
          cur_rx++;
          if (cur_rx > rx)
          {
               return cx;
          }
     }
     return cx;
}

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Searching Functions**/
//Editor Prompt Function
char *editorPrompt(char *prompt)
{
     size_t bufsize = 128;
     char *buf = malloc(bufsize);
     size_t buflen = 0;
     buf[0] = '\0';
     while (1)
     {
          editorSetStatusMessage(prompt, buf);
          refreshScreen();
          int c = editorReadKey();
          if (c == 43) // Plus key
          {
               editorSetStatusMessage("");
               free(buf);
               return NULL;
          }
          else if (c == '\r')
          {
               if (buflen != 0)
               {
                    return buf;
               }
          }
          else if (!iscntrl(c) && c < 128)
          {
               if (buflen == bufsize - 1)
               {
                    bufsize *= 2;
                    buf = realloc(buf, bufsize);
               }
               buf[buflen++] = c;
               buf[buflen] = '\0';
          }
     }
}

//To draw message bar in editor where we take search input
void editorDrawMessageBar(struct abuf *ab)
{
     abAppend(ab, "\x1b[K", 3);
     int msglen = strlen(E.statusmsg);
     if (msglen > E.screencols)
     {
          msglen = E.screencols;
     }
     if (msglen && time(NULL) - E.statusmsg_time < 5)
     {
          abAppend(ab, E.statusmsg, msglen);
     }
}

//Find Function
void editorFind()
{
     pw = calloc(N, sizeof(int));
     inv = calloc(N, sizeof(int));
     has = calloc(N, sizeof(int));
     precal();
     char *query = editorPrompt("Search: %s (+ to cancel)");
     if (query == NULL)
     {
          return;
     }
     struct ll *rowline = temphead;
     editorSetStatusMessage("%s-%s", query, rowline->data);
     rabinKarp(query, rowline->data);
     int i = 0;
     while (rowline != NULL && position == -1)
     {
          rabinKarp(rowline->data, query);
          i++;
          rowline = rowline->next;
     }
     if (rk == 1)
     {
          row_number = i;
          E.cy = row_number - 1;
          E.cx = position;
          editorSetStatusMessage("FOUND!! at %d row and %d position", row_number, position);
          rk=0;
          position=-1;
          row_number=-1;
     }
     else
     {
          editorSetStatusMessage("NOT FOUND!!");
     }
     free(query);
}

//Rabin Karp Algo
void rabinKarp(char *text, char *pattern)
{
     int hash_of_pattern = final_hash(pattern);
     int n = strlen(text);
     int m = strlen(pattern);
     build_hash(text);
     for (int i = 0; i < n - m + 1; i++)
     {
          if (hash_of_pattern == get_hash(i, i + m - 1))
          {
               rk = 1;
               position = i;
               return;
          }
     }
}

int add(int a, int b, int mod)
{
     int res = (a + b) % mod;
     if (res < 0)
     {
          res += mod;
     }
     return res;
}

int mult(int a, int b, int mod)
{
     int res = (a * 1LL * b) % mod;
     if (res < 0)
     {
          res += mod;
     }
     return res;
}

int power(int a, int b, int mod)
{
     int res = 1;
     while (b)
     {
          if (b % 2 == 1)
          {
               res = mult(res, a, mod);
          }
          a = mult(a, a, mod);
          b /= 2;
     }
     return res;
}

void precal()
{
     pw[0] = 1;
     for (int i = 1; i < N; i++)
     {
          pw[i] = mult(pw[i - 1], base, mod);
     }

     int pw_inv = power(base, mod - 2, mod);
     inv[0] = 1;
     for (int i = 1; i < N; i++)
     {
          inv[i] = mult(inv[i - 1], pw_inv, mod);
     }
}

//Building Hashing
void build_hash(char *s)
{
     int n = strlen(s);
     for (int i = 0; i < n; i++)
     {
          has[i] = add((i == 0) ? 0 : has[i - 1], mult(pw[i], s[i] - 'a' + 1, mod), mod);
     }
}

int get_hash(int x, int y)
{
     int res = add(has[y], (x == 0) ? 0 : -has[x - 1], mod);
     res = mult(res, (x == 0) ? 1 : inv[x], mod);
     return res;
}

int final_hash(char *s)
{
     int res = 0;
     int n = strlen(s);
     for (int i = 0; i < n; i++)
     {
          res = add(res, mult(pw[i], s[i] - 'a' + 1, mod), mod);
     }
     return res;
}

/**Searching functions ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Function for saving file**/

void editorSave()
{
     if (E.filename == NULL)
     {
          return;
     }
     else
     {
          struct ll *line1;
          line1 = NULL;
          int n = E.no_of_rows;
          int i = 0;
          int len;
          char s[1000];
          char filename[1000];
          while (i < n)
          {
               strcpy(s, E.row[i].chars);
               line1 = createll(line1, s);
               i++;
          }
          strcpy(filename, E.filename);
          saving_to_file(line1, filename);
          changehead(E.filename);
     }
     dirty_flag = 100;
     quit_flag = 1;
}

void saving_to_file(struct ll *head, char filename[1000])
{
     FILE *fp = fopen(filename, "w");
     while (head != NULL)
     {
          fprintf(fp, "%s\n", head->data);
          head = head->next;
     }
     fclose(fp);
}

/**Function for saving file ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Buffer functions**/

void abAppend(struct abuf *ab, const char *s, int len)
{
     char *new = realloc(ab->b, ab->len + len);
     if (new == NULL)
     {
          return;
     }
     memcpy(&new[ab->len], s, len);
     ab->b = new;
     ab->len += len;
}

void abFree(struct abuf *ab)
{
     free(ab->b);
}

/**Buffer functions ends**/

//-------------------------------------------------------------------------------------------------------------------------------------------//

/**Editor Refresh Function**/
void refreshScreen()
{
     editorScroll();

     struct abuf ab = ABUF_INIT;

     abAppend(&ab, "\x1b[?25l", 6);
     abAppend(&ab, "\x1b[H", 3);

     editorRows(&ab);
     editorDrawStatusBar(&ab);
     editorDrawMessageBar(&ab);

     char buf[32];
     snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, E.cx + 1);
     abAppend(&ab, buf, strlen(buf));

     abAppend(&ab, "\x1b[?25h", 6);
     write(STDOUT_FILENO, ab.b, ab.len);
     abFree(&ab);
}

/**Editor Refresh function ends**/

//Init editor
void initEditor()
{
     E.cx = 0;
     E.cy = 0;
     E.no_of_rows = 0;
     E.rowoff = 0;
     E.statusmsg[0] = '\0';
     E.statusmsg_time = 0;
     if (getWindowSize(&E.screenrows, &E.screencols) == -1)
     {
     die("getWindowSize");
     }
     E.screenrows -= 2;
}

/**Main function**/

int main()
{
     welcome_message();
     int code;
     char filename[1000];
     char a;
     printf("Enter the code - ");
     scanf("%d", &code);
     if (code == 1)
     {
          printf("Enter the file you want to open - ");
          scanf("%s", filename);
          FILE *fp;
          fp = fopen(filename, "r");
          if (fp == NULL)
          {
               printf("No such file exists !\n");
               exit(0);
               fclose(fp);
          }
          else
          {
               fclose(fp);
               enableRawMode();
               initEditor();
               strcpy(E.filename, filename);
               fileOpen(filename);

               while (1)
               {
                    refreshScreen();
                    editorKeypress();
               }
          }
     }

     else if (code == 2)
     {
          printf("Please enter your new file's name - ");
          scanf("%s", filename);
          FILE *fp;
          fp = fopen(filename, "r");
          if (fp == NULL)
          {
               fp = fopen(filename, "w");
               fclose(fp);
               enableRawMode();
               initEditor();
               strcpy(E.filename, filename);
               fileOpen(filename);

               while (1)
               {
               refreshScreen();
               editorKeypress();
               }
               return 0;
          }
          else
          {
               printf("File already exists!");
               fclose(fp);
          }
     }

     else
     {
          exit(0);
     }
     return 0;
}

/**main function ends**/
