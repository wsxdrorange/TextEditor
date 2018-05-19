#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

//Global Variable Declaration
struct termios orig_termios;


//Function Declaration
void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode(){ //Creates struct, modifies it, and sets attr to modified struct, allowing us to enter raw mode
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO); //turn off echo feature, input wont be repeated back to the screen
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); //TCSAflush says wait for all output to be written to term
}

int main(){
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}
