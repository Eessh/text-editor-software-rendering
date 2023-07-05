#!/bin/bash

echo Building ...
make config=release

echo Launching ...
./bin/Release/text-editor-software-rendering ./src/main.cpp