@echo off
REM [1] Don dep build cu
if exist build ( rmdir /s /q build )

REM [2] Cau hinh voi CMake
cmake -B build -G Ninja
if %ERRORLEVEL% neq 0 ( echo Configuration failed! & exit /b 1 )

REM [3] Bien dich
cmake --build build
if %ERRORLEVEL% neq 0 ( echo Build failed! & exit /b 1 )

REM [4] Copy file .hex/.elf sang thu muc Debug cho Proteus
if not exist Debug mkdir Debug
copy /Y build\Exercise_1.hex Debug\Exercise_1.hex >nul
copy /Y build\Exercise_1.elf Debug\Exercise_1.elf >nul
copy /Y build\Exercise_1.bin Debug\Exercise_1.bin >nul

REM [5] Neu co mach ST-Link cam vao va truyen tham so --flash thi nap luon
if "%1"=="--flash" (
    STM32_Programmer_CLI -c port=SWD -w build/Exercise_1.bin 0x08000000 -v -rst
)
