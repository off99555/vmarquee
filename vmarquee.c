/*
* vmarquee.c - a program for interactive marquee manipulation
*/

#include <curses.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// hopefully, user won't type anything that is longer than this maximum limit
#define MAX_STR_SIZE 1000

int nrows;
int ncols;

char QUIT_KEY = 'q';
char LEFT_KEY = 'h';
char RIGHT_KEY = 'l';
char UP_KEY = 'k';
char DOWN_KEY = 'j';
char SPEED_UP_KEY = 'f';
char SPEED_DOWN_KEY = 'b';
char TOGGLE_KEY = 't';
char INCREASE_COLSIZE_KEY = ']';
char DECREASE_COLSIZE_KEY = '[';
char RESET_COLSIZE_KEY = '$';
char INCREASE_OFFSET_KEY = '>';
char DECREASE_OFFSET_KEY = '<';
char RESET_OFFSET_KEY = '0';
char INPUT_KEY = 'e';
char DOWNWARD_KEY = 'd';
char UPWARD_KEY = 'u';

WINDOW *wnd;
int shiftval = 0; // decrement to move left, increment to move right
int ccols; // current number of columns (the blue colored space), it starts maximized
int line = 1; // current line of the text
float msdelay = 200; // time delay between marquee transitions
bool negdir = true; // will move in the negative direction if true
int offset = 0; // the column offset
char str[MAX_STR_SIZE] = "This is sample text for horizontal scrolling announcement. | "; // the marquee string
bool inputphase = false; // this indicate that the user is typing a new text
char input[MAX_STR_SIZE]; // this will store the immediate user input when he/she wants to change the str[]

// output the current time in milliseconds
clock_t timems() {
  return clock() / (CLOCKS_PER_SEC / 1000);
}

// an almost pure function you can rely that it won't change state of the *str
// and it also doesn't use any global variables
// it will copy *str and show it on the screen
// shiftval can be a really big negative/positive number
void showvmarquee(char *str, int row, int col, int colsize, int firstrow,
    int lastrow, int shiftval, bool negdir) {
  int n = strlen(str);
  char res[colsize + 1];
  for (int i = 0; i < colsize; i++) {
    if (n < colsize) { // if the window is wider than the text
      // this take care of both cases where shiftval is either big negative or big positive
      int idx = (((i + shiftval) % colsize) + colsize) % colsize;
      if (i < n)
        res[idx] = str[i];
      else
        res[idx] = ' ';
    } else { // if the window is narrow
      int idx = (((i - shiftval) % n) + n) % n;
      res[i] = str[idx]; 
    }
  }
  res[colsize] = 0; // null-terminated character
  int fgcolor = negdir ? COLOR_RED : COLOR_GREEN;
  int bgcolor = negdir ? COLOR_YELLOW : COLOR_BLUE;
  start_color();
  init_pair(1, fgcolor, bgcolor); // set foreground and background color
  attron(COLOR_PAIR(1));
  int r = row;
  int c = col;
  for (int i = 0; i < colsize; i++) {
    mvaddch(r, c, res[i]);
    if (++r > lastrow) {
      r = firstrow;
      c++;
    }
  }
  attroff(COLOR_PAIR(1));
}

// redraw everything on the screen while using some global variables
void update() {
  static int lastncols = 0; // keeps track of the last known window size
  getmaxyx(wnd, nrows, ncols);

  // trying to reset everything if the window is resized
  if (lastncols != ncols) {
    lastncols = ncols;
    ccols = ncols;
    offset = 0;
  }
  clear();

  // text stuff on the bottom of the window
  char msg[MAX_STR_SIZE]; // a temporary str for formatting purpose only
  sprintf(msg, "Marquee delay: %.3f ms || Marquee Speed: %.3f characters per second", msdelay, 1000.0f / msdelay);
  mvaddstr(nrows-1, 0, msg);
  sprintf(msg, "Column Size: %d / %d", ccols, ncols);
  mvaddstr(nrows-2, 0, msg);
  sprintf(msg, "Column Offset: %d / %d", offset, ncols - 1);
  mvaddstr(nrows-3, 0, msg);
  sprintf(msg, "Keys: '%c' scrolls negative, '%c' scrolls positive, '%c' scrolls up, '%c' scrolls down,\n '%c' speeds up, '%c' speeds down, '%c' quit, '%c' toggle direction,\n '%c' increase column size, '%c' decrease column size, '%c' reset column size,\n '%c' increase offset, '%c' decrease offset, '%c' reset offset\n '%c' down direction, '%c' up direction",
      LEFT_KEY, RIGHT_KEY, UP_KEY, DOWN_KEY, SPEED_UP_KEY, SPEED_DOWN_KEY, QUIT_KEY, TOGGLE_KEY, INCREASE_COLSIZE_KEY, DECREASE_COLSIZE_KEY, RESET_COLSIZE_KEY, INCREASE_OFFSET_KEY, DECREASE_OFFSET_KEY, RESET_OFFSET_KEY, DOWNWARD_KEY, UPWARD_KEY);
  mvaddstr(nrows-9, 0, msg);
  char header[] = "== RESPONSIVE MARQUEE MANIPULATOR ==";
  // some little arithmetic to make the text appear in the middle of the screen
  mvaddstr(0, ncols/2 - strlen(header)/2, header);
  // we can be sure that this function won't change any global variables like most badly designed code do
  showvmarquee(str, line, offset, ccols, 1, nrows-11, shiftval, negdir);
  if (inputphase) {
    mvaddstr(nrows-4, 0, "Type '=' to confirm your new input");
    mvaddstr(nrows-10, 0, ": ");
    addstr(input);
  } else {
    sprintf(msg, "Type '%c' to start entering new input", INPUT_KEY);
    mvaddstr(nrows-4, 0, msg);
  }
  refresh();
}

void resetColsize() {
  ccols = nrows-12;
  update();
}
int main(int argc, char **argv) {
  wnd = initscr();
  cbreak();
  noecho();
  //timeout(1000);
  nodelay(wnd, true); // allow non-blocking IO
  update();

  clock_t last, now;
  last = timems();
  while (true) {
    char btn = getch();
    if (inputphase) {
      // You cannot type punctuations. Only English characters, spaces, and digits are allowed.
      if ((btn >= 'A' && btn <= 'Z') || (btn >= 'a' && btn <= 'z') || (btn >= '0' && btn <= '9') || btn == ' ') {
        int n = strlen(input);
        input[n] = btn;
        input[n+1] = 0;
        update();
      } else if (btn == '=') { // I don't know how to check for Enter key, I've tried KEY_ENTER but it doesn't work, so I used equal key instead.
        strcpy(str, input);
        inputphase = false;
      }
    } else {
      // the idea of this block is simeple: adjust global variables based on user input
      // and then call update() to redraw because update() use global variables
      if (btn == QUIT_KEY)
        break;
      if (btn == LEFT_KEY) {
        shiftval--;
        update();
      } else if (btn == RIGHT_KEY) {
        shiftval++;
        update();
      } else if (btn == UP_KEY) {
        line--;
        if (line < 1)
          line = 1;
        update();
      } else if (btn == DOWN_KEY) {
        line++;
        if (line > nrows - 10)
          line = nrows - 10;
        update();
      } else if (btn == SPEED_UP_KEY) {
        msdelay /= 1.2;
        update();
      } else if (btn == SPEED_DOWN_KEY) {
        msdelay *= 1.2;
        update();
      } else if (btn == TOGGLE_KEY) {
        negdir = !negdir;
      } else if (btn == INCREASE_COLSIZE_KEY) {
        ccols++;
        update();
      } else if (btn == DECREASE_COLSIZE_KEY) {
        ccols--;
        if (ccols < 1)
          ccols = 1;
        update();
      } else if (btn == INCREASE_OFFSET_KEY) {
        offset++;
        if (offset > ncols - 1)
          offset = ncols - 1;
        update();
      } else if (btn == DECREASE_OFFSET_KEY) {
        offset--;
        if (offset < 0)
          offset = 0;
        update();
      } else if (btn == RESET_OFFSET_KEY) {
        offset = 0;
        update();
      } else if (btn == RESET_COLSIZE_KEY) {
        resetColsize();
      } else if (btn == INPUT_KEY) {
        inputphase = true;
        input[0] = 0;
        update();
      } else if (btn == DOWNWARD_KEY) {
        negdir = false;
        update();
      } else if (btn == UPWARD_KEY) {
        negdir = true;
        update();
      }
    }

    // on each loop, this will do nothing until the condition is met
    // doesn't block I/O like sleep
    now = timems();
    clock_t diff = now - last;
    if (diff >= msdelay) {
      if (negdir)
        shiftval--;
      else
        shiftval++;
      update();
      last += msdelay;
    }
  }

  endwin();
  return EXIT_SUCCESS;
}
