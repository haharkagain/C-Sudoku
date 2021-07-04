g++ main.c SDL2.dll
if exist "a.exe" (
	a.exe
	del a.exe
) else (
	pause
)