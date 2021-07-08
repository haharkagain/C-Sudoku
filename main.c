#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_thread.h"
#include <stdio.h>
#include <string.h>

int init();         // start SDL and create a window, returns 0 on error
void close();		// deallocate memory and close the window

int load();			// preload all the necessary textures for the renderer, returns 0 on error
SDL_Texture* loadNumber(int num);	// load a number from a bmp image

void draw();		// renders the textures into the window
void drawThickLine(int thickness, int startX, int startY, int endX, int endY);	// draw a thicker line

int eventHandler(SDL_Event e);	// handle events
void copyBoard(int dest[9][9], int src[9][9]);	// copies the sudoku board from src into dest

int solveSudoku(int array[9][9]);	// solve the sudoku recursively
int findCell(int array[9][9], int* row, int* col);	// find an empty cell, return 0 if everything is full
int validVal(int array[9][9], int val, int row, int col);	// check value at position, return 1 if valid, 0 if invalid

int solveThread(void* data); // thread for solveSudoku

SDL_Window* window = NULL;		// window 

SDL_Renderer* renderer = NULL;	// renderer

SDL_Texture* numbers[10];		// array of textures for each number

const int SCREEN_WIDTH = 720, SCREEN_HEIGHT = SCREEN_WIDTH;	// screen resolution
int unit = SCREEN_WIDTH / 9;	// unit used for drawing
int posX = 0, posY = 0;			// position of selector

enum DrawMode {		// enum for drawing while solving
	DrawMode_OFF,
	DrawMode_ONCE,
	DrawMode_ON
};
int drawMode = DrawMode_OFF;

int workingBoard[9][9];
int initBoard[9][9] = {
	{0, 0, 0, 8, 0, 1, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 4, 3, 0},
	{5, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 7, 0, 8, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 2, 0, 0, 3, 0, 0, 0, 0},
	{6, 0, 0, 0, 0, 0, 0, 7, 5},
	{0, 0, 3, 4, 0, 0, 0, 0, 0},
	{0, 0, 0, 2, 0, 0, 6, 0, 0}
};
/*
	{8, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 3, 6, 0, 0, 0, 0, 0},
	{0, 7, 0, 0, 9, 0, 2, 0, 0},
	{0, 5, 0, 0, 0, 7, 0, 0, 0},
	{0, 0, 0, 0, 4, 5, 7, 0, 0},
	{0, 0, 0, 1, 0, 0, 0, 3, 0},
	{0, 0, 1, 0, 0, 0, 0, 6, 8},
	{0, 0, 8, 5, 0, 0, 0, 1, 0},
	{0, 9, 0, 0, 0, 0, 4, 0, 0}

	{0, 0, 0, 8, 0, 1, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 4, 3, 0},
	{5, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 7, 0, 8, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 2, 0, 0, 3, 0, 0, 0, 0},
	{6, 0, 0, 0, 0, 0, 0, 7, 5},
	{0, 0, 3, 4, 0, 0, 0, 0, 0},
	{0, 0, 0, 2, 0, 0, 6, 0, 0}
*/

void WinMain(int arg, char *argv[]) {
    if (!init()) return;	// initialize the video, window, and renderer
	if (!load()) return;	// load the textures
	copyBoard(workingBoard, initBoard);	// set the working board
	draw();			// initial draw (otherwise only draws on a keyDown event)
	SDL_Event e;	// event struct
	int quit = 0;	// quit flag
	while (!quit) {	// game loop
		if (SDL_PollEvent(&e)) {	// poll for events
			quit = eventHandler(e);		// handle the event, returns 1 to quit		
		}
		if (drawMode) {		// drawing while solving
			draw();
			if (drawMode == DrawMode_ONCE) {	// draw after finished solving
				drawMode = DrawMode_OFF;
				draw();
			}
		}
	}
    close();		// quit the program
    return;			
}

int init() {	// start SDL and create a window, returns 0 on error
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {		// video init
        printf("MeepMop: %s\n", SDL_GetError());
        return 0;
    } 
    window = SDL_CreateWindow("yo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {							// window init
        printf("MeepMop: %s\n", SDL_GetError());
        return 0;
    }
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {						// renderer init
		printf("MeepMop: %s\n", SDL_GetError());
		return 0;
	}
    return 1;
}

int load() {	// preload all the necessary textures for the renderer, returns 0 on error
	int i;
	for (i=1; i <= 9; i++) {
		numbers[i] = loadNumber(i);	// load the number textures
		if (numbers[i] == NULL) {	// catch errors
			return 0;
		}
	}
	return 1;
}

SDL_Texture* loadNumber(int num) {		// load a number from a bmp image
   	char end[5], mid[6], path[13];

   	strcpy(path, "assets/");			// folder with the images
   	strcpy(end, ".bmp");				// filetype
	itoa(num, mid, 10);					// number of the file

   	strcat(mid, end);					// puts the filetype onto the number
	strcat(path, mid);					// puts the file onto the folder path

	SDL_Surface* image = SDL_LoadBMP(path);		// load the bmp
	if (!image) {								// catch errors
		printf("MeepMop: %s\n", SDL_GetError());
		return NULL;
	}
	SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF));	// colorkey the whitespace
	return SDL_CreateTextureFromSurface(renderer, image);	// return the texture
}	

void draw() {	// renders the textures into the window
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);	// white
	SDL_RenderClear(renderer);									// draw background
	
	if (!drawMode) {	// don't draw current position while solving
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);	// highlight current position
		SDL_Rect drawUnit = {posX * unit, posY * unit, unit, unit};
		SDL_RenderFillRect(renderer, &drawUnit);
	}
	
	SDL_SetRenderDrawColor(renderer, 0xC0, 0xC0, 0xC0, 0xFF);	// light gray
	int i, j;	// i is row, j is column
	for (i=0; i < 9; i++) {		
		for (j=0; j < 9; j++) {
			if (workingBoard[i][j]) {	// draw the numbers on the board
				SDL_Rect drawUnit = {j * unit, i * unit, unit, unit};
				if (initBoard[i][j] && ((posY != i) ||(posX != j))) {
					SDL_RenderFillRect(renderer, &drawUnit);	// gray unit for initial values
				}
				SDL_RenderCopy(renderer, numbers[workingBoard[i][j]], NULL, &drawUnit);
			}
		}
	}

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);	// black
	for (i=1; i < 9; i++) {		// draw the grid
		if (i % 3 == 0) {		// every third line is thick
			drawThickLine(5, i * SCREEN_WIDTH/9, 0, i * SCREEN_WIDTH/9, SCREEN_HEIGHT);
			drawThickLine(5, 0, i * SCREEN_HEIGHT/9, SCREEN_WIDTH, i * SCREEN_HEIGHT/9);
		} else {				
			SDL_RenderDrawLine(renderer, i * SCREEN_WIDTH/9, 0, i * SCREEN_WIDTH/9, SCREEN_HEIGHT);
			SDL_RenderDrawLine(renderer, 0, i * SCREEN_HEIGHT/9, SCREEN_WIDTH, i * SCREEN_HEIGHT/9);
		}
	}
	SDL_RenderPresent(renderer);	// put the render onto the window
	return;
}

void drawThickLine(int thickness, int startX, int startY, int endX, int endY) {	// draw a thicker line
	int i;
	if (endY-startY) {	// make exception for when the slope would otherwise be vertical
		int slope = -(endX-startX) / (endY-startY);	// for a straight and thick line edge
		for (i=0; i < thickness; i++) {
			SDL_RenderDrawLine(renderer, startX++, startY+=slope, endX++, endY+=slope);
		}
	} else {	// vertical slope, only make new lines vertically
		for (i=0; i < thickness; i++) {
			SDL_RenderDrawLine(renderer, startX, startY++, endX, endY++);
		}
	}
	return;
}

int eventHandler(SDL_Event e) {	// handle events
	if (e.type == SDL_QUIT) {	// quit event
		return 1;
	} else if(e.type == SDL_KEYDOWN) {	// keydown event
		if (drawMode) return 0;			// dont take events while solving
		switch(e.key.keysym.sym) {
			case SDLK_UP:				// up
				if (posY > 0) posY--; 	// check if on the edge of the board
				break;
			case SDLK_DOWN:				// down
				if (posY < 8) posY++;	
				break;
			case SDLK_LEFT:				// left
				if (posX > 0) posX--;
				break;
			case SDLK_RIGHT:			// right
				if (posX < 8) posX++;
				break;
			case SDLK_BACKSPACE:		// backspace
			case SDLK_0:				// 0
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 0;	// dont change initial values
				break;
			case SDLK_2:				// 1
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 2;
				break;
			case SDLK_1:				// 2
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 1;
				break;
			case SDLK_3:				// 3
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 3;
				break;
			case SDLK_4:				// 4
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 4;
				break;
			case SDLK_5:				// 5
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 5;
				break;
			case SDLK_6:				// 6
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 6;
				break;
			case SDLK_7:				// 7
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 7;
				break;
			case SDLK_8:				// 8
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 8;
				break;
			case SDLK_9:				// 9
				if (!initBoard[posY][posX]) workingBoard[posY][posX] = 9;
				break;
			case SDLK_RETURN:			// enter
				SDL_CreateThread(solveThread, "solveSudoku", NULL);		// creates a new thread for solving
				break;
			case SDLK_BACKSLASH:		// backslash
				copyBoard(workingBoard, initBoard);		// reset the board
				break;
			default:
				return 0;
		}
		draw();
	}
	return 0;
}

void copyBoard(int dest[9][9], int src[9][9]) {
	int i, j;
	for (i=0; i < 9; i++) {
		for (j=0; j < 9; j++) {
		 	dest[i][j] = src[i][j];
		}
	}
	return;
}

int solveThread(void* data) {	// calls solveSudoku then draws when done
	drawMode = DrawMode_ON;		// for drawing in the main function
	copyBoard(workingBoard, initBoard);
	solveSudoku(workingBoard);	
	drawMode = DrawMode_ONCE;	// one last draw in the main function after finishing
	return 0;
}

void close() {		// deallocate memory and close the window	
	int i;	
	for (i=1; i < 9; i++) {			
		SDL_DestroyTexture(numbers[i]);	// free number textures
		numbers[i] = NULL;
	}
	SDL_DestroyRenderer(renderer);	// free renderer
	renderer = NULL;
    SDL_DestroyWindow(window);		// free window
    window = NULL;
    SDL_Quit();						// quit
}

int findCell(int array[9][9], int* row, int* col) {	// find an empty cell, return 0 if everything is full
	int i, j;	// go through the board
	for (i=0; i < 9; i++) {	
		for (j=0; j < 9; j++) {
			if (!array[i][j]) {	// if empty cell
				*row = i;	// set the coordinates at the 
				*col = j;	// value in the given pointers
				return 1;	
			}
		}
	}
	return 0;	// return 0 if all cells populated
}

int validVal(int array[9][9], int val, int row, int col) {	// check value at position, return 1 if valid, 0 if invalid
	int i, j;
	for (i=0; i < 9; i++) {		// check the rows and the columns
		if (array[i][col] == val) return 0;
		if (array[row][i] == val) return 0;
	}
	col -= col % 3;				// set the coordinate to the top right corner of the given 3x3 area
	row -= row % 3;
	for (i=row; i < row+3; i++) {	// check the 3x3 area
		for (j=col; j < col+3; j++) {
			if (array[i][j] == val) return 0;
		}
	}
	return 1;
}

int solveSudoku(int array[9][9]) {	// solve the sudoku recursively
	int row = 0, col = 0;
	if (!findCell(array, &row, &col)) return 1;	// base case when all cells are filled, sets position otherwise
	int val;
	for (val=1; val <= 9; val++) {	// try every value at position
		if (validVal(array, val, row, col)) {	// check if valid at position
			array[row][col] = val;	// set value at position
			if (solveSudoku(array)) return 1;	// recursive call
			array[row][col] = 0;	// backtracking
		}
	}
	return 0;
}