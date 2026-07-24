@echo off
setlocal

set "TEST_DIR=%~dp0."
set "BUILD_DIR=%~dp0build_tests"

cmake -S "%TEST_DIR%" -B "%BUILD_DIR%"
if errorlevel 1 exit /b 1

cmake --build "%BUILD_DIR%" --config Debug
if errorlevel 1 exit /b 1

ctest --test-dir "%BUILD_DIR%" -C Debug --output-on-failure
if errorlevel 1 exit /b 1

echo All module CLI tests passed.
