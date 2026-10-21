@echo off
setlocal

if "%~1" == "" (
    echo error: Missing input file
    goto usage_error
)

if "%~1" == "--transpiler-only" (
    set build_transpiler_only=1
    if "%~2" == "" (
        echo error: Missing CC
        goto usage_error
    )
    set CC=%~2
    set CFLAGS=%~3
) else (
    if "%~2" == "" (
        echo error: Missing output file
        goto usage_error
    )
    if "%~3" == "" (
        echo error: Missing CC
        goto usage_error
    )
    set build_transpiler_only=
    set in=%~1
    set out=%~2
    set CC=%~3
    set CFLAGS=%~4
)

echo Using:
echo CC=%CC%
echo CFLAGS=%CFLAGS%


echo Using CC=%CC%
echo Using CFLAGS=%CFLAGS%

cmd /c "echo("
echo Building bootstrap transpiler
echo %CC% %CFLAGS% bootstrap-transpiler.c -o build/bootstrap-transpiler.exe
%CC% %CFLAGS% bootstrap-transpiler.c -o build/bootstrap-transpiler || exit /b 1

if "%build_transpiler_only%" == "" (
    cmd /c "echo("
    echo Building %in%

    echo build\bootstrap-transpiler.exe %in% %out%.c
    build\bootstrap-transpiler.exe %in% %out%.c || exit /b 1

    echo %CC% %CFLAGS% %out%.c -o %out%
    %CC% %CFLAGS% %out%.c -o %out% || exit /b 1
)

exit /b 0

:usage_error
echo Usage:
echo cmd /c build.bat in.yezik build/out [CC] [CFLAGS]
exit /b 1

endlocal
