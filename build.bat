@echo off

SET "START_PROJ=test"

REM Set the path to the Visual Studio generator
SET "generator=Visual Studio 17 2022"

REM Set the path to the source and build directories
SET "build=./build"

REM Check if the argument "clean" is provided
IF "%1"=="clean" (
    rmdir /s /q "%build%"
    exit /b
)

IF "%1"=="run" (
    REM Run the generated executable
    "%build%/%START_PROJ%/Release/%START_PROJ%.exe"
    exit /b
)

cls
REM Generate the Visual Studio solution using CMake
cmake -G "%generator%" -B "%build%"

REM Build the solution using MSBuild
cmake --build "%build%" --config Release

Pause
