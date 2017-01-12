# CMakeList du bas niveau INTech
# compilation pour STM32F407VG
#
#
# Le fichier stm32_flash.ld doit être placé dans le même dossier que le Makefile généré :
# ~/.CLion2016.2/system/cmake/generated/complete_low_level-3d133f2f/3d133f2f/Debug
#

cmake_minimum_required(VERSION 3.5)
project(arm)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)


SET(CMAKE_CXX_COMPILER arm-none-eabi-g++)
SET(CMAKE_C_COMPILER arm-none-eabi-gcc)

SET(CMAKE_C_FLAGS "-g -O2 -Wall -Tstm32_flash.ld -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -I." CACHE INTERNAL "c compiler flags")
SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -Tstm32_flash.ld -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -I." CACHE INTERNAL "c compiler flags")



#SET(CMAKE_CXX_FLAGS "--specs=nosys.specs -mthumb -lstdc++ -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wall -std=c++11 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "cxx compiler flags")
SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "executable linker flags")
SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "module linker flags")
SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "shared linker flags")
#SET(STM32_CHIP_TYPES 405xx 415xx 407xx 417xx 427xx 437xx 429xx 439xx 446xx 401xC 401xE 411xE CACHE INTERNAL "stm32f4 chip types")
#SET(STM32_CODES "405.." "415.." "407.." "417.." "427.." "437.." "429.." "439.." "446.." "401.[CB]" "401.[ED]" "411.[ED]")



include_directories("cmsis"
        "cmsis_boot"
        "cmsis_boot/startup"
        "cmsis_lib/include"
        "include"
        "library"
        "stm32_discovery_arm_gcc/STM32F4-Discovery_FW_V1.1.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/")

set(SOURCE_FILES
    cmsis/core_cm4.h
    cmsis/core_cm4_simd.h
    cmsis/core_cmFunc.h
    cmsis/core_cmInstr.h
    cmsis_boot/startup/startup_stm32f4xx.c
    cmsis_boot/stm32f4xx.h
    cmsis_boot/stm32f4xx_conf.h
    cmsis_boot/system_stm32f4xx.c
    cmsis_boot/system_stm32f4xx.h
    cmsis_lib/include/misc.h
    cmsis_lib/include/stm32f4xx_adc.h
    cmsis_lib/include/stm32f4xx_exti.h
    cmsis_lib/include/stm32f4xx_gpio.h
    cmsis_lib/include/stm32f4xx_rcc.h
    cmsis_lib/include/stm32f4xx_rtc.h
    cmsis_lib/include/stm32f4xx_syscfg.h
    cmsis_lib/include/stm32f4xx_tim.h
    cmsis_lib/include/stm32f4xx_usart.h
    cmsis_lib/source/misc.c
    cmsis_lib/source/stm32f4xx_adc.c
    cmsis_lib/source/stm32f4xx_exti.c
    cmsis_lib/source/stm32f4xx_gpio.c
    cmsis_lib/source/stm32f4xx_rcc.c
    cmsis_lib/source/stm32f4xx_rtc.c
    cmsis_lib/source/stm32f4xx_syscfg.c
    cmsis_lib/source/stm32f4xx_tim.c
    cmsis_lib/source/stm32f4xx_usart.c
        library/average.hpp
        library/ax12.hpp
        library/capteur_srf05.hpp
        library/delay.c
        library/delay.h
        library/ltoa.c
        library/ltoa.h
        library/pid.hpp
        library/ring_buffer.hpp
        library/safe_enum.hpp
        library/Singleton.hpp
        library/Uart.hpp
        library/Uart_IT.cpp
        library/utils.h
        library/utils.c
        library/voltage_controller.hpp
    include/ActuatorsMgr.hpp
    include/Counter.h
    include/MotionControlSystem.h
    include/Motor.h
    include/SensorMgr.h
    src/Counter.cpp
    src/MotionControlSystem.cpp
    src/Motor.cpp
    src/SensorMgr.cpp
    syscalls/syscalls.c
    main.cpp src/Elevator.cpp include/Elevator.h src/ElevatorMgr.cpp include/ElevatorMgr.h)

add_executable(complete_low_level ${SOURCE_FILES})