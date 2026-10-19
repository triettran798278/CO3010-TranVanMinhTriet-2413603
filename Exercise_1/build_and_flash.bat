@echo off
setlocal
set "PATH=C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin;C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.500.202603051304\tools\bin;C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cmake.win32_1.1.200.202605190741\tools\bin;C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.ninja.win32_1.1.200.202606260906\tools\bin;%LOCALAPPDATA%\Programs\MinGit\cmd;%PATH%"
cd /d "%~dp0"

echo ==============================================================
for /f "tokens=*" %%b in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set "CURRENT_BRANCH=%%b"
echo [BRANCH HIEN TAI]: %CURRENT_BRANCH%
echo ==============================================================

REM [1] Don dep build cu
echo [1/4] Don dep thu muc build cu...
if exist build ( rmdir /s /q build )

REM [2] Cau hinh voi CMake
echo [2/4] Cau hinh CMake + Ninja...
cmake -B build -G Ninja
if %ERRORLEVEL% neq 0 (
    echo [LOI] Configuration failed!
    pause
    exit /b 1
)

REM [3] Bien dich
echo [3/4] Bien dich source code hien tai...
cmake --build build
if %ERRORLEVEL% neq 0 (
    echo [LOI] Build failed!
    pause
    exit /b 1
)

REM [4] Xuat file .hex/.elf/.bin sang Debug cho Proteus
echo [4/4] Cap nhat file Exercise_1.hex cho Proteus...
if not exist Debug mkdir Debug
copy /Y build\Exercise_1.hex Debug\Exercise_1.hex >nul
copy /Y build\Exercise_1.elf Debug\Exercise_1.elf >nul
copy /Y build\Exercise_1.bin Debug\Exercise_1.bin >nul

echo.
echo ==============================================================
echo [THANH CONG] Da build xong branch [%CURRENT_BRANCH%]!
echo  - File HEX cho Proteus: %~dp0Debug\Exercise_1.hex
echo  - File Proteus        : C:\Users\Triet\Documents\Year3\VXL\lab1.pdsprj
echo ==============================================================

if "%1"=="--flash" (
    STM32_Programmer_CLI -c port=SWD -w build/Exercise_1.bin 0x08000000 -v -rst
)

