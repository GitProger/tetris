/*************************
 * Tetris
 * C89 - Ansi C
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

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
setpos(int x, int y) { /* set cursor position on the screen at x,y */
   COORD C = {x, y};
   SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), C);
}

unsigned int random() { /* a random number */
     static int is_init = 0;
     static unsigned long long x = 0;
     if (!is_init) {
         x = time(NULL);
         is_init = 1;
     }    
     x = (0x343FDULL * x + 0x269EC3ULL) % 0xFFFFFFFFULL;
     return (x & 0x3FFF8000) >> 15;
}

void setcur(bool mode) { /* turn on/off cursor */
   HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_CURSOR_INFO structCursorInfo;
   GetConsoleCursorInfo(handle, &structCursorInfo);
   structCursorInfo.bVisible = mode ? TRUE : FALSE;
   SetConsoleCursorInfo(handle, &structCursorInfo);
}

/* == Data == */
bool figure[4][4] = { }, _next_fig[4][4] = { }; 
unsigned char map[10][20]  = { }; /* lines [0-9][20-22] are buffer */
int  CurFigX = 3, CurFigY = -3; /* coord of up left corner of figure */ 
int  CurFigType = 0, NextFigType = 0, NextFigID, score = 0, lines = 0;
const bool /* list of figyres */
__Fig0[4][4] = {
  {0, 0, 0, 0},
  {0, 1, 1, 0},
  {0, 1, 1, 0},
  {0, 0, 0, 0}
},
__Fig1[4][4] = {
  {0, 1, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 0, 0}
},
__Fig2[4][4] = {
  {0, 1, 1, 0},
  {0, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 0, 0}
},
__Fig3[4][4] = {
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 0}
},
__Fig4[4][4] = {
  {0, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 0},
  {0, 0, 0, 0}
},
__Fig5[4][4] = {
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 0}
},
__Fig6[4][4] = {
  {0, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 0, 0}, 
  {0, 1, 0, 0}
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
     if (CurFigType == (BACKGROUND_INTENSITY|BACKGROUND_RED)) return;
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
             if (buffer[x][y] && (CurFigX + x > 9 || CurFigY + y > 19))
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
void Draw2(void);
void Generate(void);

int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
    GetConsoleScreenBufferInfo(h, &ConsoleInfo);
    WORD OriginalColors = ConsoleInfo.wAttributes; 
   
    int i, x, y, points;
    char key;
    bool quick = false;
    NextFigID = random() % 7 + 1;
    Generate();
    start:
      score = lines = 0;
      printf("Press any key..."); 
      getch();
      for (x = 0; x < 10; x++)
          for (y = 0; y < 20; y++)
              map[x][y] = 0;

    for (;;) {
      setcur(false);
      key = 0;
      for (i = 0; i < (quick ? 10000 : 25000); i++)
          if (kbhit()) {
              key = getch(); 
          }
      quick = false;
      if (key == 27 ) break; else
      if (key == ' ') getch(); else
      if (key == 'w') Rotate(); else
      if (key == 'a') ShiftLeft(); else
      if (key == 'd') ShiftRight(); else
      if (key == 's') quick = true; else
      if (key =='\r') while (ShiftDown()); else
           switch (key) { /* arraw is 2 chars */
             case 72: Rotate();    break; 
             case 75: ShiftLeft(); break; 
             case 77: ShiftRight();break; 
             case 80: quick = true;        
           }
          
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
          printf("     Game over!     ");
          getchar();
          goto start;
      }
    }
    setcur(true);
    SetConsoleTextAttribute(h, OriginalColors);
    return 0;
}

void FigCpy(bool a[4][4], const bool b[4][4]) {
     int x, y;
     for (y = 0; y < 4; y++)
        for (x = 0; x < 4; x++)
            a[x][y] = b[x][y];
}

void Generate() { /* generate next figure */
     switch (NextFigID) {
         case 1: FigCpy(figure, __Fig0); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_RED); break;
         case 2: FigCpy(figure, __Fig1); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_RED  | BACKGROUND_GREEN); break;
         case 3: FigCpy(figure, __Fig2); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED); break;
         case 4: FigCpy(figure, __Fig3); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE); break;
         case 5: FigCpy(figure, __Fig4); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN); break;
         case 6: FigCpy(figure, __Fig5); CurFigType = (BACKGROUND_INTENSITY | BACKGROUND_GREEN); break;
         case 7: FigCpy(figure, __Fig6); CurFigType = (BACKGROUND_RED | BACKGROUND_GREEN); 
     }
     NextFigID = random() % 7 + 1;
     switch (NextFigID) {
         case 1: FigCpy(_next_fig, __Fig0); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_RED); break;
         case 2: FigCpy(_next_fig, __Fig1); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_RED  | BACKGROUND_GREEN); break;
         case 3: FigCpy(_next_fig, __Fig2); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED); break;
         case 4: FigCpy(_next_fig, __Fig3); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE); break;
         case 5: FigCpy(_next_fig, __Fig4); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN); break;
         case 6: FigCpy(_next_fig, __Fig5); NextFigType = (BACKGROUND_INTENSITY | BACKGROUND_GREEN); break;
         case 7: FigCpy(_next_fig, __Fig6); NextFigType = (BACKGROUND_RED | BACKGROUND_GREEN); 
     }
}

void rect(int x, int y, int color) { /* put a 1x1 rectangle at x,y */
     if (x < 0 || y < 0) return;
     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
     DWORD cWritten;
     const char fill_char = ' ';
     SetConsoleTextAttribute(h, color);
     setpos(x, y);
     WriteFile(h, &fill_char, 1, &cWritten, NULL);
     SetConsoleTextAttribute(h, 0);
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

             SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 
                BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
             setpos(20, y);
             printf("                    ");
             Sleep(50);
             SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN);

             DelLine(y);
         }
     }
     return lines;
}

void Draw() { /* draw map, figure, border, next figure, scores */
     int x, y;
     HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
     SetConsoleTextAttribute(h, BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN);
     static bool done = false;
     if (!done) {
          done = true;
          system("cls");
          printf("Score: 0\nHigh score: 0\nLines: 0");
          for (y = 0; y < 20; y++) {
             setpos(19, y); putchar('|');
             setpos(40, y); putchar('|');
          }
          setpos(19, 20); 
          for (x = 0; x < 22; x++) putchar('-');
     }
     if (gethighscore() < score) setscr(score);
     setpos(7, 0); printf("%d", score);
     setpos(12, 1); printf("%d", gethighscore());
     setpos(7, 2); printf("%d", lines);
     SetConsoleTextAttribute(h, 0);
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
     SetConsoleTextAttribute(h, 0);
     for (y = 0; y < 4; y++) {
         for (x = 0; x < 4; x++) {
              square(x + 1, y + 4, NextFigType * (int)_next_fig[x][y]);
         }
     }
  /* = next figure = */
     SetConsoleTextAttribute(h, BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN);
}

