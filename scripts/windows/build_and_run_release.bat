@echo off

mingw32-make config=release

echo:
echo Launching...
.\bin\Release\text-editor-software-rendering.exe .\src\main.cpp

@REM echo:
@REM echo Press enter to exit
@REM set /p input=
exit