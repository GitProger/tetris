#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <curses.h>
#include <time.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
int getch() {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
int kbhit() {
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void setscr(int a) { /* write the high score to a file */ 
     FILE *f = fopen("data.bin", "wb");
     fwrite(&a, sizeof(a), 1, f);
     fclose(f);
}
int gethighscore() { /* read the high score from a file */
     int r = 0;
     FILE *f = fopen("data.bin","rb");
     if (!f)
         setscr(0);
     else
         fread(&r, sizeof(r), 1, f); 
     fclose(f);
     return r;
}
void setpos(int x, int y) { /* set cursor position on the screen at x,y */
    printf("%c[%d;%df", 0x1B, y + 1, x + 1);
}

void setcol(int id) {
    char cl[16];
    if (id)
        sprintf(cl, "\e[1;4%dm", id);
    else
        sprintf(cl, "\e[0m");
    printf("%s", cl);
}

unsigned int trandom() { /* a random number */
     static int is_init = 0;
     static unsigned long long x = 0;
     if (!is_init) {
         x = time(NULL);
         is_init = 1;
     }    
     x = (0x343FDULL * x + 0x269EC3ULL) % 0xFFFFFFFFULL;
     return (x & 0x3FFF8000) >> 15;
}

/* == Data == */
bool figure[4][4] = { }, _next_fig[4][4] = { }; 
unsigned char map[10][20]  = { }; /* lines [0-9][20-22] are buffer */
int  CurFigX = 3, CurFigY = -3; /* coord of up left corner of figure */ 
int  CurFigType = 0, NextFigType = 0, NextFigID, score = 0, lines = 0;
const bool __Fig[7][4][4] = {/* list of figyres */
  {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
  },
  {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0}, 
    {0, 1, 0, 0}
  }
};
/* == Data == */

bool IsLost() { /* check is game possible */
     int x, y;
     for (x = 3; x < 7; x++)
             if (map[x][0])
                 return true; 
     return false;
}

void Rotate() { /* rotate current figure */
     if (CurFigType == 1 /*square*/) return;
     int x, y, emptylines = 0;
     bool is3x3 = true, buffer[4][4] = { };
 /* = if figure isn`t 4x4 or 2x2 but 3x3 = */
     for (y = 0; y < 4; y++) 
         if (figure[3][y]) 
             is3x3 = false;
     for (x = 0; x < 4; x++) 
         if (figure[x][3]) 
             is3x3 = false;
 /* = ================================== = */
     if (is3x3) {
         for (x = 0; x < 3; x++)
             for (y = 0; y < 3; y++)
                 buffer[x][y] = figure[y][2-x];
     } else {
         for (x = 0; x < 4; x++)
             for (y = 0; y < 4; y++)
                 buffer[x][y] = figure[y][3-x];
     }
 /* = can we rotate?                     = */
     for (x = 0; x < 4; x++) {
         for (y = 0; y < 4; y++) {
             if (buffer[x][y] && map[CurFigX + x][CurFigY + y])
                 return;
         }
     }
 /* = ================================== = */
 /* = corner? = */
     for (x = 0; x < 4; x++) {
         for (y = 0; y < 4; y++) {
             if (buffer[x][y] && 
                     (CurFigX + x > 9  || CurFigX + x < 0) ||
                     (CurFigY + y > 19 || CurFigY + y < 0)
             )
                 return;
         }
     }
    
    /* = ======= = */
     for (x = 0; x < 4; x++)
         for (y = 0; y < 4; y++)
             figure[x][y] = buffer[x][y];
}

void ShiftRight() { /* shift current figure right */
     int x, y, lastline = 3; /* last empty vertical */
     bool empty = true, possible = true;
     for (x = 3; x > -1; x--) {
         for (y = 0; y < 4; y++)
             if (figure[x][y])
                empty = false;
         if (empty) lastline--;
     }
         if (CurFigX + 3 != 9)
             for (x = 0; x < 4; x++)
                 for (y = CurFigY; y < CurFigY + 4; y++)
                     if (figure[x][y - CurFigY] && map[CurFigX+x+1][y])
                         possible = false;

     if (CurFigX < 9 - lastline && possible)
        CurFigX++;
}

void ShiftLeft() { /* shift current figure left */
     int x, y, lastline = 0; /* first empty vertical */ 
     bool empty = true, possible = true;
     for (x = 0; x < 4; x++) {
         for (y = 0; y < 4; y++)
             if (figure[x][y])
                empty = false;
         if (empty) lastline++;
     }
        if (CurFigX > 0) 
             for (x = 0; x < 4; x++)
                 for (y = CurFigY; y < CurFigY + 4; y++)
                     if (figure[x][y - CurFigY] && map[CurFigX-1+x][y]) 
                         possible = false;

     if (CurFigX > 0 - lastline && possible)
         CurFigX--;
}

bool ShiftDown() { /* return opportunity shifting down current figure, if possible shift */
     bool empty = true, possible = true;
     int x, y, emptylines = 0; /* so we should check last line of "figure" */
        for (y = 3; y > -1; y--) {
            for (x = 0; x < 4; x++) {
                if (figure[x][y]) {
                     empty = false;
                     break;
                }
            }
            if (!empty) break; 
            emptylines++;
        }
        for (y = 0; y < 4; y++)
             for (x = CurFigX; x < CurFigX + 4; x++)
                 if (CurFigY >= -1) /* else we will can compare with map[x][CurFigY+y+1] (map[x][y-2]) and y < 2 it will be last 19`th line */
                 if (figure[x - CurFigX][y] && map[x][CurFigY+y+1]) {
                     possible = false;
                     break;
                 }
     if (CurFigY < 16 + emptylines && possible) /* 19 - emptylines */
         CurFigY++;
     else return false;
     return true;
}

int  CheckLine(void);
void DrawFigOnMap(void);
void Draw(void);
void DrawScr(void);
void Generate(void);

int main() {
    int i, x, y, points;
    char key;
    bool quick = false;
    NextFigID = random() % 7 + 1;
    Generate();
    system("setterm -cursor off");
    start:
      score = lines = 0;
      printf("Press any key...");
      getch();
      for (x = 0; x < 10; x++)
          for (y = 0; y < 20; y++)
              map[x][y] = 0;
      DrawScr();
    for (;;) {
      key = 0;
      for (i = 0; i < (quick ? 10000 : 50000); i++)
          if (kbhit())
              key = getch(); 
      quick = false;
      if (key == '\033') {
          if (getch() == '[') {
              switch (getch()) { /* arraw is 2 chars */
                  case 'A': Rotate();     break; 
                  case 'D': ShiftLeft();  break; 
                  case 'C': ShiftRight(); break; 
                  case 'B': quick = true;        
              }
          }
      } else
      if (key == ' ') getch(); else
      if (key == 'w') Rotate(); else
      if (key == 'a') ShiftLeft(); else
      if (key == 'd') ShiftRight(); else
      if (key == 's') quick = true; else
      if (key =='\n') while (ShiftDown());
          
      if (!ShiftDown()) {
          DrawFigOnMap();
          CurFigX = 3;
          CurFigY = -3;
          lines += (points = CheckLine());
          if (points) 
              score += 2 << points;
          else score++;
          Generate();
      }

      Draw();
      if (IsLost()) {
          setpos(20, 10);
          printf("\e[0m");
          printf("     Game over!     ");
          getchar();
          goto start;
      }
    }
    system("setterm -cursor on");
    return 0;
}

void FigCpy(bool a[4][4], const bool b[4][4]) {
     int x, y;
     for (y = 0; y < 4; y++)
        for (x = 0; x < 4; x++)
            a[x][y] = b[x][y];
}

void Generate() { /* generate next figure */
     FigCpy(figure, __Fig[NextFigID - 1]);
     CurFigType = NextFigID;
     NextFigID = trandom() % 7 + 1; /* 0 is empty ceil */
     FigCpy(_next_fig, __Fig[NextFigID - 1]);
     NextFigType = NextFigID;
}

void rect(int x, int y, int color) { /* put a 1x1 rectangle at x,y */
     if (x < 0 || y < 0) return;
     setpos(x, y);
     setcol(color);
     putchar(' ');
}

void square(int x, int y, int color) { /* squre at x,y */
   rect(2 * x    , y, color);
   rect(2 * x + 1, y, color);
}

void DrawFigOnMap() { /* Draw figure on map before generating a new */
     int x, y;
     for (y = CurFigY; y < CurFigY + 4; y++) {
         for (x = CurFigX; x < CurFigX + 4; x++) {
             if (figure[x - CurFigX][y - CurFigY] && x < 10 && x >= 0 && y < 20 && y >= 0)
                 map[x][y] = CurFigType * (int)figure[x - CurFigX][y - CurFigY];
         }
     }
}

void DelLine(int Y) { /* delete line at Y position */
     int x, y;
     for (x = 0; x < 10; x++) {
         for (y = Y; y > 0; y--) {
              map[x][y] = map[x][y-1];
         }
     }
     for (x = 0; x < 10; x++) 
         map[x][0] = 0;
}

int CheckLine() {  /* returns amount of deleted lines */
     int x, y, lines = 0;
     bool full;
     for (y = 0; y < 20; y++) {
         full = true;
         for (x = 0; x < 10; x++)
             if (!map[x][y]) {
                 full = false;
                 break;
             }
         if (full) {
             lines++;
             setcol(6);
             setpos(20, y);
             printf("                    ");
             usleep(100);
             DelLine(y);
         }
     }
     return lines;
}

void DrawScr() {
     int x, y;
     printf("\e[0m");
     system("clear");
     printf("Score: 0\r\nHigh score: 0\r\nLines: 0\r\n");
     for (y = 0; y < 20; y++) {
         setpos(19, y); putchar('|');
         setpos(40, y); putchar('|');
     }
     setpos(19, 20); 
     for (x = 0; x < 22; x++) putchar('-');
}
void Draw() { /* draw map, figure, border, next figure, scores */
     int x, y;
     printf("\e[0m");

     if (gethighscore() < score) setscr(score);
     setpos(7 , 0); printf("%d", score);
     setpos(12, 1); printf("%d", gethighscore());
     setpos(7 , 2); printf("%d", lines);
  /* = draw map = */
     for (y = 0; y < 20; y++) {
         for (x = 0; x < 10; x++) {
              square(10 + x, y, map[x][y]);
         }
     } 
  /* = draw map = */
  /* = draw figure = */
     for (y = CurFigY; y < CurFigY + 4; y++) {
         for (x = CurFigX; x < CurFigX + 4; x++) {
             if (figure[x - CurFigX][y - CurFigY]) /* if ceil in figure is black that means that it is clear ceil */
                 square(10 + x, y, CurFigType);
         }
     }
  /* = draw figure = */
  /* = next figure = */
     for (y = 0; y < 4; y++) {
         for (x = 0; x < 4; x++) {
              square(x + 1, y + 4, NextFigType * (int)_next_fig[x][y]);
         }
     }
  /* = next figure = */
}
