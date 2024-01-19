cmake_minimum_required(VERSION 3.10)


set(TOOLCHAIN_PREFIX "/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64")
set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_PREFIX}/bin")

set(CMAKE_C_COMPILER    /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-gcc CACHE PATH "")
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_BIN_DIR}/msp430-elf-g++")
set(CMAKE_AR            "${TOOLCHAIN_BIN_DIR}/msp430-elf-ar")
set(CMAKE_LINKER        /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-ld CACHE PATH "")
set(CMAKE_NM            "${TOOLCHAIN_BIN_DIR}/msp430-elf-nm")
set(CMAKE_OBJDUMP       "${TOOLCHAIN_BIN_DIR}/msp430-elf-objdump")
set(CMAKE_STRIP         "${TOOLCHAIN_BIN_DIR}/msp430-elf-strip")
set(CMAKE_RANLIB        "${TOOLCHAIN_BIN_DIR}/msp430-elf-ranlib")
set(CMAKE_SIZE          "${TOOLCHAIN_BIN_DIR}/msp430-elf-size")

#
# Compiler flags.
# See https://gcc.gnu.org/onlinedocs/gcc/MSP430-Options.html
# and https://www.ti.com/lit/ug/slau646f/slau646f.pdf
#

# Set MCU target
set(MCU_SPECIFIC_CFLAGS "-mmcu=msp430fr5994")
# Set type of hardware multiply supported by the target.
string(APPEND MCU_SPECIFIC_CFLAGS " -mhwmult=f5series")
# Use large-model addressing
string(APPEND MCU_SPECIFIC_CFLAGS " -mlarge")
# Toolchain header files
string(APPEND MCU_SPECIFIC_CFLAGS " -I${TOOLCHAIN_PREFIX}/msp430-elf/sys-include")

string(APPEND MCU_SPECIFIC_CFLAGS " -mcode-region=none")
string(APPEND MCU_SPECIFIC_CFLAGS " -mdata-region=none")
# string(APPEND MCU_SPECIFIC_CFLAGS " -D__MSP430__" "-D__MSP430FR5994__")

set(CMAKE_C_FLAGS "${MCU_SPECIFIC_CFLAGS}" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -gdwarf-2 -ggdb -gstrict-dwarf" CACHE INTERNAL "c compiler flags debug")
set(CMAKE_C_FLAGS_RELEASE "-Os -flto -DNDEBUG" CACHE INTERNAL "c compiler flags release")

#
# Linker flags.
#
# This path contains default linker scripts
set(MCU_SPECIFIC_LINKER_FLAGS "-L${TOOLCHAIN_PREFIX}/msp430-elf/sys-include ")
set(CMAKE_EXE_LINKER_FLAGS "${MCU_SPECIFIC_LINKER_FLAGS} -Wl,--gc-sections -Wl,-lgcc -Wl,-lc -Wl,-Map=bootloader.map" CACHE STRING "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(foo)

add_executable(bootloader.out
    main.c
    lora/lora.c
)