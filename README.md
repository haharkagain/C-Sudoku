# C-Sudoku
sudoku written in c using sdl2. includes a recursive solver.

to run:
  1) have a 64 bit windows system (untested on other systems, just get the proper include and library 
     from https://www.libsdl.org/download-2.0.php and it should work)
  2) have g++ (i used mingw)
  3) run start.bat
  4) enjoy

use the arrow keys to move the position of the highlighted square.
to set a number at the position of the yellow square, enter a number 1-9. 
to clear a square, press '0' at the position.
if you want the sudoku to be solved, press 'enter' for the recursive algorithm to begin.

its pretty barebones, but it works twice as fast as my javascript version, despite the identical algorithms. 
prolly because c is closer to machine code. brendan eich is in shambles.
