@echo off

setlocal

set EXE_NAME=main.exe

set OUT_DIR=build
set OUT_DIR_BIN=%OUT_DIR%\bin
set OUT_DIR_OBJ=%OUT_DIR%\obj

set "C_FLAGS=/nologo"
set "C_FLAGS=%C_FLAGS% /std:c++latest /utf-8 /FC /EHsc"
set "C_FLAGS=%C_FLAGS% -W4 -wd4127 -wd4146 -wd4505 -wd4456 -wd4457"
set "C_FLAGS=%C_FLAGS% /DWIN32 /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /DGLAD_GL_IMPLEMENTATION"
set "C_FLAGS=%C_FLAGS% /DNDEBUG /D_NDEBUG"
set "C_FLAGS=%C_FLAGS% /I Vendor\glad\include /I Vendor\glfw\include"

set "L_FLAGS=/link"
set "L_FLAGS=%L_FLAGS% /LIBPATH:vendor\glew\lib /LIBPATH:vendor\glfw\lib"
set "L_FLAGS=%L_FLAGS% /INCREMENTAL:no"
set "L_FLAGS=%L_FLAGS% glfw3.lib opengl32.lib"
set "L_FLAGS=%L_FLAGS% user32.lib gdi32.lib shell32.lib"
set "L_FLAGS=%L_FLAGS% /NODEFAULTLIB:LIBCMT /ENTRY:mainCRTStartup"
set "L_FLAGS=%L_FLAGS% /IGNORE:4099"

set "TARGET_MODE=%~1"
if "%TARGET_MODE%"=="" set "TARGET_MODE=debug"

if /i "%TARGET_MODE%"=="debug" (
    set "MODE=[DEBUG]"
    set "C_FLAGS=%C_FLAGS% /Od /MD /Zi"
) else if /i "%TARGET_MODE%"=="release" (
    set "MODE=[RELESE]"
    set "C_FLAGS=%C_FLAGS% /O2 /MD /GF"
    set "L_FLAGS=%L_FLAGS% /SUBSYSTEM:windows"
) else if /i "%TARGET_MODE%"=="clean" (
    if exist %OUT_DIR% (
        rd /s /q %OUT_DIR% && echo [CLEAN] Removed: %OUT_DIR%
    )
    exit /b 0
) else (
    echo build.bat ["" or debug^|release^|clean]
    exit /b 1
)

if not exist %OUT_DIR% mkdir %OUT_DIR%
if not exist %OUT_DIR_BIN% mkdir %OUT_DIR_BIN%
if not exist %OUT_DIR_OBJ% mkdir %OUT_DIR_OBJ%

cl src\main.cc /Fd"%OUT_DIR_OBJ%\\" /Fo"%OUT_DIR_OBJ%\\" /Fe"%OUT_DIR_BIN%\\%EXE_NAME%" %C_FLAGS% %L_FLAGS%
if %ERRORLEVEL% neq 0 exit /b 1

for /f %%i in ('cmd /k prompt $e^<nul') do set ESC=%%i

echo %ESC%[1;32m%MODE%%ESC%[0m An executable file has been generated: %ESC%[1m%OUT_DIR%\bin\%EXE_NAME%%ESC%[0m

endlocal
