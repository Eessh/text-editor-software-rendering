#!/bin/bash

echo Building ...
make config=debug

echo Launching ...
./bin/Debug/text-editor-software-rendering ./src/main.cpp