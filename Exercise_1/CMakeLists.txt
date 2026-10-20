cmake_minimum_required(VERSION 3.22)

set(CMAKE_PROJECT_NAME Exercise_1)
include("cmake/gcc-arm-none-eabi.cmake")

project(${CMAKE_PROJECT_NAME})
enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME})

file(GLOB HAL_SOURCES "Drivers/STM32F1xx_HAL_Driver/Src/*.c")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Src/main.c
    Src/software_timer.c
    Src/seg7.c
    Src/matrix.c
    Src/stm32f1xx_it.c
    Src/stm32f1xx_hal_msp.c
    Src/system_stm32f1xx.c
    Src/syscalls.c
    Src/sysmem.c
    Startup/startup_stm32f103c6ux.s
    ${HAL_SOURCES}
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F103x6
    HSI_VALUE=8000000
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Inc
    Drivers/STM32F1xx_HAL_Driver/Inc
    Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F1xx/Include
    Drivers/CMSIS/Include
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)
