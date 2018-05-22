#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>

//Definitions
#define CTRL_KEY(k) ((k) & 0x1f)

//Global Variable Declaration
struct editorConfig{
    struct termios orig_termios;

    int screenrows;
    int screencols;
};

struct editorConfig E;

//Prints error and exits program
void die(const char *s){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}


//Function Declaration
//Terminal Functions
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
        die("tcsetattr");
    }
}
void enableRawMode(){ //Creates struct, modifies it, and sets attr to modified struct, allowing us to enter raw mode
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1){
        die("tcgetattr");
    }
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
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
char editorReadKey(){ //Read in key
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1){
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}
int getCursorPosition(int *rows, int *cols){
    char buf[32];
    unsigned int i = 0;

    if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) -1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
    }
    buf[i] = '\0';

    if(buf[0] != '\x1b' || buf[1] != '[') return -1;
    if(sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}
int getWindowSize(int *rows, int*cols){ //Get size terminal window 
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        editorReadKey();
        return getCursorPosition(rows, cols);
    } 
    else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
//Output
void editorDrawRows(){ //draws tildes on each row after the last edited portion
    int i;
    for (i = 0; i < E.screenrows; i++){
        write(STDOUT_FILENO, "~", 1);
        if(i < E.screenrows -1){
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
}
void editorRefreshScreen(){ //Positions cursor and clears contents
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}
//Input
void editorProcessKeypress(){
    char c = editorReadKey();
    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/* INIT */
void initEditor(){
    if(getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(){
    enableRawMode();    
    initEditor();    

    while (1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
