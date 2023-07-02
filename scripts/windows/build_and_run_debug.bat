@echo off

mingw32-make config=debug

echo:
echo Launching...
.\bin\Debug\text-editor-software-rendering.exe

@REM echo:
@REM echo Press enter to exit
@REM set /p input=
exit