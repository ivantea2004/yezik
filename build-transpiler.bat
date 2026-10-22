@echo off
setlocal

if "%~1" == "" (
    echo [CC] is missing
    call :print_usage
    exit /b 1
)

set CC=%~1
set CFLAGS=%~2

echo CC=%CC%
echo CFLAGS=%CFLAGS%

if %CC:~-4% == .exe (
    set CC=%CC:~0,-4%
)

if %CC:~-2% == cl (
    echo Using cl.exe command line syntax
    set CFLAGS=%CFLAGS% /Fo:build\ /Fd:build\ /Fe:
) else (
    echo Using GNU command line syntax
    set CFLAGS=%CFLAGS% -o
)

cmd /c "echo("
echo Building build/transpiler.exe
echo %CC% %CFLAGS% build/transpiler.exe transpiler.c
%CC% %CFLAGS% build/transpiler.exe transpiler.c || exit /b 1

endlocal
goto :eof

:print_usage
    echo Usage:
    echo cmd /c build-transpiler.bat [CC] [CFLAGS]
exit /b 1
