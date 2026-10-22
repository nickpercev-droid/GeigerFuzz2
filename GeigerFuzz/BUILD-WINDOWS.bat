@echo off
setlocal
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1
cmake --build build --config Release --target GeigerFuzz_VST3
if errorlevel 1 exit /b 1
echo.
echo Build finished. Look in build\GeigerFuzz_artefacts\Release\VST3\
pause
