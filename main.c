#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>

struct termios orig_termios;

void DisableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void RawMode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(DisableRawMode);

    //To disable raw mode at exit.

    struct termios raw=orig_termios;
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag != (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    //timeout for read()
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(){
    //command to enable raw mode
    RawMode();

    //read() will read 1 byte from input and store in variable c 
    char c;
    while(1){
        char c='\0';
        read(STDIN_FILENO, &c, 1);
        if(iscntrl(c)){
            printf("%d\r\n",c);
        }
        else{
            printf("%d ('%c')\r\n",c,c);
        }
        if(c=='q'){
            break;
        }
    }

    DisableRawMode();
    return 0;
}
