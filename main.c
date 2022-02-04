#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

struct ll{
  struct ll *next;
  char data[1000];
};

//Doubly linked list
//Datatype for storing  a row
typedef struct row{
  int size;
  char *chars;
  struct row *prev;
  struct row *next;
}row;

/**define**/

#define PRATIK_PATIL_VERSION "104"
#define CTRL_KEY(k) ((k)& 0x1f)

enum editorKey {
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY
};

struct termios orig_termios;

//global struct which will contain width and height of terminal
struct editorConfig{
  int cx, cy;
  int screenrows;
  int screencols;
  int no_of_rows;
  row *row;
  struct termios orig_termios;
  char filename[1000];
}E;


struct ll* createll(struct ll* head,char A[1000]){
  struct ll *first,*t,*last;
  first=head;
  last=first;
  if(first==NULL){
    first=(struct ll*)malloc(sizeof(struct ll));
    first->next=NULL;
    strcpy(first->data,A);
    last=first;
  }
  else{
    while(last->next!=NULL){
      last=last->next;
    }
    t=(struct ll*)malloc(sizeof(struct ll));
    t->next=NULL;
    last->next=t;
    strcpy(t->data,A);
  }
  return first;
}

/**terminal**/
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode() { 
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios)==-1)
        die("tcsetattr");
}

void enableRawMode() 
{
    // tcgetattr(STDIN_FILENO, &orig_termios);
    if(tcgetattr(STDIN_FILENO, &E.orig_termios)==-1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(ICRNL| INPCK |ISTRIP| IXON | BRKINT);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |=(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1)
        die ("tcsetattr");
 }

int editorReadKey(){
  int r;
  char c;
  while((r=read(STDIN_FILENO,&c,1))!=1){
    if(r==-1 && errno != EAGAIN) 
        //EAGAIN - there is no data available
        die("read");
  }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': return ARROW_UP;
        case 'B': return ARROW_DOWN;
        case 'C': return ARROW_RIGHT;
        case 'D': return ARROW_LEFT;
      }
    }
    return '\x1b';
  } else {
    return c;
  }
}

int getWindowSize(int *rows, int *cols){
  struct winsize ws;

  if(ioctl(STDOUT_FILENO,TIOCGWINSZ, &ws)==-1||ws.ws_col==0){
    return -1;
  }
  //on failure it will return -1

  else{
    *cols=ws.ws_col;
    *rows=ws.ws_row;
  }
}

void editorAppendRow(char *s,size_t len){
  E.row = realloc(E.row, sizeof(row) * (E.no_of_rows + 1));
  int at = E.no_of_rows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.no_of_rows++;

  /*E.row.size=len;
  E.row.chars=malloc(len+1);
  memcpy(E.row.chars,s,len);
  E.row.chars[len]='\0';
  E.no_of_rows++;*/
}

void editorRowInsertChar(row *row, int at, int c){
  if(at<0 || at>row->size){
    at=row->size;
  }
  row->chars = realloc(row->chars,row->size+2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  //editorUpdateRow(row);
}

void editorInsertChar(int c) {
  if (E.cy == E.no_of_rows) {
    editorAppendRow("", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}

//file i/o
char *editorRowsToString(int *buflen) {
  int totlen = 0;
  int j;
  for (j = 0; j < E.no_of_rows; j++)
    totlen += E.row[j].size + 1;
  *buflen = totlen;
  char *buf = malloc(totlen);
  char *p = buf;
  for (j = 0; j < E.no_of_rows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }
  return buf;
}

void editorOpen(struct ll *head){
  while(head!=NULL){
    editorAppendRow(head->data,strlen(head->data));
    head=head->next;
  }
}

void editorSave() {
  if (E.filename == NULL) return;
  int len;
  char *buf = editorRowsToString(&len);
  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        return;
      }
    }
    close(fd);
  }
  free(buf);
}

void fileOpen(char filename[1000]){
  FILE *fptr;
  fptr=fopen(filename,"r");
  if(fptr==NULL){
    exit(0);
  }
  char c=fgetc(fptr);
  int i=0;
  char s[1000]="";
  char s1[1000]="";
  int n=0;
  struct ll *head;
  head=NULL;
  while(c!=EOF){
    if(c=='\n'){
      c=fgetc(fptr);
      head=createll(head,s);
      //editorOpen(s,strlen(s));
      strcpy(s,s1);
      n++;
    }
    else{
      strncat(s,&c,1);
      c=fgetc(fptr);
    }
  }
  editorOpen(head);
  fclose(fptr);
}

//append buffer
struct abuf{
  char *b;
  int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len){
  char *new = realloc(ab->b, ab->len+len);

  if(new==NULL) return;
  memcpy(&new[ab->len],s,len);
  ab->b=new;
  ab->len+=len;
}

void abFree(struct abuf *ab){
  free(ab->b);
}

/***output***/
void editorRows(struct abuf *ab){
  int y;
  for(y=0;y<E.screenrows;y++){
    if(y>=E.no_of_rows){
      if(y==E.screenrows/3){
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),"Desperate times desperate measures %s", PRATIK_PATIL_VERSION);
        if (welcomelen > E.screencols){
          welcomelen = E.screencols;
        }
        int padding = (E.screencols - welcomelen) / 2;
        if (padding) {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      }
      else{
        abAppend(ab, "~", 1);
      }
    }
    else{
      int len = E.row[y].size;
      if(len>E.screencols){
        len=E.screencols;
      }
      abAppend(ab,E.row[y].chars,len);
    }

    abAppend(ab, "\x1b[K", 3);

    //for getting tilde on last line
    if(y<E.screenrows-1){
      abAppend(ab, "\r\n", 2);
    }
  }
}

//re-render editor interface after each key press
void refreshScreen(){
  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorRows(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);
  //4 - means : writing 4 bytes to the terminal 
  /***
   \x1b - first : 27 in decimal
   [2J - next three
   escape sequences always start with \x1b
   <esc>[2J : clear entire screen
   ***/
  //repositioning the cursor to the top
  //H - to reposition the cursor
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

/***input***/

void editorMoveCursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      }
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screencols - 1) {
        E.cx++;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenrows - 1) {
        E.cy++;
      }
      break;
  }
}

void editorKeypress(){
  int c = editorReadKey();

  switch(c){
    case '\r':
      /* TODO */
      break;

    case CTRL_KEY('s'):
      editorSave();
      break;

    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    
    case BACKSPACE:
      /* TODO */
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

/**init**/

void initEditor(){
  E.cx = 0;
  E.cy = 0;
  E.no_of_rows=0;

  if(getWindowSize(&E.screenrows,&E.screencols)==-1){
    die("getWindowSize");
  }
}


int main() {
  char filename[1000];
  printf("Enter the file you want to open - ");
  scanf("%s",filename);
  int num;
  printf("Enter 1 to view or press 2 to type something - ");
  scanf("%d",&num);
  if(num==1){
    enableRawMode();
    initEditor();
    fileOpen(filename);
    
    while(1){
      refreshScreen();
      editorKeypress();
    }
  }
  if(num==2){
    enableRawMode();
    initEditor();
    
    while(1){
      refreshScreen();
      editorKeypress();
    }
  }
  return 0;
}