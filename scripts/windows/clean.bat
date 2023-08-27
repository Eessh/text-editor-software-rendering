@echo off

echo Cleaning distributables ...
del /q dist\windows\*.exe >nul 2>&1

echo Cleaning distributable config ...
del /q dist\windows\config.toml >nul 2>&1

echo Cleaning distributable assets ...
rmdir /s /q dist\windows\assets

echo Cleaning compiled resources ...
del /q res.res >nul 2>&1

echo Cleaning executables ...
del /q bin\Debug\*.exe >nul 2>&1
del /q bin\Release\*.exe >nul 2>&1

echo Cleaning object files ...
del /q obj\Debug\* >nul 2>&1
del /q obj\Release\* >nul 2>&1

echo Cleaning misc files ...
del /q Makefile >nul 2>&1
del /q *.make >nul 2>&1

echo Done.

exit

