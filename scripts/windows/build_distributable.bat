@echo off

echo Compiling resources (windows)...
windres res.rc -O coff -o res.res

echo Building distributable (windows)...
g++ src\*.cpp .\log-boii\*.c^
	-I .\cairo-windows-1.17.2\include^
	-I .\freetype^
	-I .\SDL2-2.26.5\x86_64-w64-mingw32\include^
	-L .\cairo-windows-1.17.2\lib\x64^
	-L .\freetype\lib\x86_64^
	-L .\SDL2-2.26.5\x86_64-w64-mingw32\lib^
	-I .\include^
	-I .\log-boii^
	-lSDL2main -lSDL2^
	-lcairo^
	-lfreetype^
	-lmingw32^
	-mwindows .\res.res^
	-O3 -o .\dist\windows\Rocket.exe

echo:
echo Done.
echo Rocket app is here: dist/windows/Rocket.exe

@REM echo:
@REM echo Press enter to exit
@REM set /p input=
exit

