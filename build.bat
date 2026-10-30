@echo off
echo ========================================
echo   CSCI 4110 PROJECT BUILD SYSTEM
echo ========================================

:: Navigate to the project root
set ROOT_DIR=%~dp0
pushd %ROOT_DIR%

:: Safety check for source files
if not exist src\main.cpp (
    echo [ERROR] I couldn't find src\main.cpp!
    echo Current directory: %cd%
    echo Make sure all your .cpp files are inside the 'src' folder.
    popd
    pause
    exit /b
)

echo Compiling the Fish Sim from \src... 

:: The main build command. 
:: /O2: This is the "Optimization" flag. It makes the O(N^2) loops in the Boids math run significantly faster.
:: /Zi: Generates debug info just in case of a crash.
cl /EHsc /MD /O2 /Zi src\*.cpp tiny_obj_loader.cc glad/src/glad.c ^
    /I"src" /I"glad/include" /I"glfw/include" /I"." ^
    /link /LIBPATH:"glfw/lib-vc2022" glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib ^
    /OUT:FishSchoolingSimulation.exe

:: Check for success
if %ERRORLEVEL% EQU 0 (
    echo ========================================
    echo [SUCCESS] Build made :]
    echo Just run: FishSchoolingSimulation.exe
    echo ========================================
) else (
    echo [ERROR] Something went wrong. Check the errors in the terminal.
)

popd
pause