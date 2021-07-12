@echo off

@REM Generate Assimp Project with CMake
echo Generating Assimp Project files with CMake
echo ===================================================
pushd %~dp0\Pelican\dependencies\assimp\
mkdir build
call cmake -B .\build\ -S .\
popd

echo.

@REM Generate all other project files with Premake
echo Generating project files with Premake
echo ===================================================
call premake5.exe vs2019 --use-vld

pause
