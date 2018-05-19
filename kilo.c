#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

//Global Variable Declaration
struct termios orig_termios;

//Prints error and exits program
void die(const char *s){
    perror(s);
    exit(1);
}


//Function Declaration
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
        die("tcsetattr");
    }
}
void enableRawMode(){ //Creates struct, modifies it, and sets attr to modified struct, allowing us to enter raw mode
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1){
        die("tcgetattr");
    }
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_cflag &= ~(CS8);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); //turn off echo feature, input wont be repeated back to the screen
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;    

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) { //TCSAflush says wait for all output to be written to term
        die("tcsetattr");
    }
}


/* INIT */
int main(){
    enableRawMode();    

    char c;
    while (1){
        c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
            die("read");
        }
        if(iscntrl(c)){
            printf("%d\r\n", c);
        }
        else{
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}
