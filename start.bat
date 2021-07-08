@ECHO OFF
if exist "main.exe" (
	del main.exe
)
gcc main.c SDL2.dll -o main.exe -O3 -Wall
if exist "main.exe" (
	echo Compiled, running...
	main.exe
	del main.exe
) else (
	echo.
	pause
)