#!/bin/bash

echo Formatting code ...
clang-format -i -style=file include/*.hpp src/*.cpp

echo Done.