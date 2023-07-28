@echo off

echo Compiling resources (windows)...
windres res.rc -O coff -o res.res

echo Building distributable (windows). This may take some time...
g++ src\*.cpp log-boii\*.c cpp-tokenizer\*.cpp^
	-I .\cairo-windows-1.17.2\include^
	-I .\freetype^
	-I .\SDL2-2.26.5\x86_64-w64-mingw32\include^
	-L .\cairo-windows-1.17.2\lib\x64^
	-L .\freetype\lib\x86_64^
	-L .\SDL2-2.26.5\x86_64-w64-mingw32\lib^
	-I .\include^
	-I .\log-boii^
	-I .\cpp-tokenizer^
	-lSDL2main -lSDL2^
	-lcairo^
	-lfreetype^
	-lmingw32^
	-mwindows .\res.res^
	-O3 -o .\dist\windows\Rocket.exe

echo Copying config ...
copy config.toml dist\windows\config.toml

echo:
echo Done.
echo Rocket app is here: dist/windows/Rocket.exe

@REM echo:
@REM echo Press enter to exit
@REM set /p input=
exit

