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
  char *render;
  int rsize;
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

//Displaying starting message
void welcome_message(){
    printf("\n");
    printf("\t\t\t\t\t\t\t  Welcome to PRATIK'S TEXT EDITOR ! \n\n");
    printf("\t\t\t\t\t\t\t    Please see codes given below\n");
    printf("\n");
    printf("\t\t\t\t -------------------------------------------------------------------------------------- \n");
    printf("\t\t\t\t|                            |                            |                            |\n");
    printf("\t\t\t\t|");
    printf("        READ A FILE         ");
    printf("|");
    printf("     CREATE A NEW FILE      ");
    printf("|");
    printf("    OPEN AN EXISTING FILE   ");
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
void editorUpdateRow(row *row) {
  free(row->render);
  row->render = malloc(row->size + 1);
  int j;
  int idx = 0;
  for (j = 0; j < row->size; j++) {
    row->render[idx++] = row->chars[j];
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorRowDelChar(row *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row); 
}

void editorDelChar() {
  if (E.cy == E.no_of_rows) return;
  row *row = &E.row[E.cy];
  if (E.cx > 0) {
    editorRowDelChar(row, E.cx - 1);
    E.cx--;
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

  E.row[at].rsize = 0;
  E.row[at].render = NULL;

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
/*char *editorRowsToString(int *buflen) {
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
}*/

void editorOpen(struct ll *head){
  while(head!=NULL){
    editorAppendRow(head->data,strlen(head->data));
    head=head->next;
  }
}

void saving_to_file(struct ll *head,char filename[1000]){
  FILE *fp=fopen(filename,"w");
  while(head!=NULL){
    fprintf(fp,"%s\n",head->data);
    head=head->next;
  }
  fclose(fp);
}

void editorSave() {
  if (E.filename == NULL) return;
  else{
    struct ll* line1;
    line1=NULL;
    int n=E.no_of_rows;
    int i=0;
    int len;
    char s[1000];
    char filename[1000];
    while(i<n){
      strcpy(s,E.row[i].chars);
      line1=createll(line1,s);
      i++;
    }
    strcpy(filename,E.filename);
    saving_to_file(line1,filename);
  }
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
void editorDrawStatusBar(struct abuf *ab) {
  abAppend(ab, "\x1b[7m", 4);
  char status[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines",
    E.filename ? E.filename : "[No Name]", E.no_of_rows);
  if (len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);
  while (len < E.screencols) {
    abAppend(ab, " ", 1);
    len++;
  }
  abAppend(ab, "\x1b[m", 3);
}

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
    abAppend(ab, "\r\n", 2);
    
  }
}

//re-render editor interface after each key press
void refreshScreen(){
  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorRows(&ab);
  editorDrawStatusBar(&ab);

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

/**init**/

void initEditor(){
  E.cx = 0;
  E.cy = 0;
  E.no_of_rows=0;

  if(getWindowSize(&E.screenrows,&E.screencols)==-1){
    die("getWindowSize");
  }
  E.screenrows -= 1;
}


int main() {
  welcome_message();
  int code;
  char filename[1000];
  char a;
  printf("Enter the code - ");
  scanf("%d",&code);
  if(code==1){
    printf("Enter the file you want to read - ");
    scanf("%s",filename);
    FILE *fp;
    fp=fopen(filename,"r");
    if(fp==NULL){
      printf("No such file exists !\n");
      fclose(fp);
      exit(0);
    }
    else{
      fclose(fp);
      enableRawMode();
      initEditor();
      strcpy(E.filename,filename);
      fileOpen(filename);
    
      while(1){
        refreshScreen();
        editorKeypress();
      }
    }
  }
  else if(code==2){
    printf("Please enter your new file's name - ");
    scanf("%s",filename);
    FILE *fp;
    fp=fopen(filename,"r");
    if(fp==NULL){
      fp=fopen(filename,"w");
      fclose(fp);
      enableRawMode();
      initEditor();
      strcpy(E.filename,filename);
      fileOpen(filename);
    
      while(1){
        refreshScreen();
        editorKeypress();
      }
      return 0;
    }
    printf("File with this name already exists !\n");
    printf("Do you want to replace it ? (Y/N) - ");
    scanf("%c",&a);
    
      //fclose(fp);
      
      if(a=='Y'){
        fclose(fopen(filename, "w"));
        fclose(fp);
        enableRawMode();
        initEditor();
        strcpy(E.filename,filename);
        fileOpen(filename);
    
        while(1){
          refreshScreen();
          editorKeypress();
        }
      }
      else{
        printf("Okay new file was not created !");
        exit(0);
      }
    
  }
  // char filename[1000];
  // printf("Enter the file you want to open - ");
  // scanf("%s",filename);
  // int num;
  // printf("Enter 1 to view or press 2 to type something - ");
  // scanf("%d",&num);
  // if(num==1){
  //   enableRawMode();
  //   initEditor();
  //   strcpy(E.filename,filename);
  //   fileOpen(filename);
    
  //   while(1){
  //     refreshScreen();
  //     editorKeypress();
  //   }
  // }
  // if(num==2){
  //   strcpy(E.filename,filename);
  //   enableRawMode();
  //   initEditor();
    
  //   while(1){
  //     refreshScreen();
  //     editorKeypress();
  //   }
  // }
}

