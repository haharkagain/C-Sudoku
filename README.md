# C-Sudoku
sudoku written in c using sdl2. includes a recursive solver.

to run:
  1) have a 64 bit windows system (untested on other systems, just get the proper include and library 
     from https://www.libsdl.org/download-2.0.php and it should work)
  2) have g++ (i used mingw)
  3) run start.bat (if you're not on windows put "g++ main.c SDL2.dll" in the terminal)
  4) enjoy

- 'arrowkeys' to move
- '1-9' to set a number at the selected position
- '0' or 'backspace' to clear the number at the selected position
- 'enter' to start the solver
- '\\' to reset

its pretty barebones, but it works twice as fast as my javascript version, despite the identical algorithms. 
prolly because c is closer to machine code. brendan eich is in shambles.
