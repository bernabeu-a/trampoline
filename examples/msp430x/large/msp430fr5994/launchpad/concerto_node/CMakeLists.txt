cmake_minimum_required(VERSION 3.10)

set(TOOLCHAIN_PREFIX "/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64")
set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_PREFIX}/bin")

set(CMAKE_C_COMPILER    /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-gcc CACHE PATH "")
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_BIN_DIR}/msp430-elf-g++")
set(CMAKE_AR            "${TOOLCHAIN_BIN_DIR}/msp430-elf-ar")
set(CMAKE_ASM_COMPILER  /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-gcc CACHE PATH "")
set(CMAKE_AS_COMPILER  /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-gcc CACHE PATH "")
set(CMAKE_AS            /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-as CACHE PATH "")
set(CMAKE_LINKER        /home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-ld CACHE PATH "")
set(CMAKE_NM            "${TOOLCHAIN_BIN_DIR}/msp430-elf-nm")
set(CMAKE_OBJDUMP       "${TOOLCHAIN_BIN_DIR}/msp430-elf-objdump")
set(CMAKE_STRIP         "${TOOLCHAIN_BIN_DIR}/msp430-elf-strip")
set(CMAKE_RANLIB        "${TOOLCHAIN_BIN_DIR}/msp430-elf-ranlib")
set(CMAKE_SIZE          "${TOOLCHAIN_BIN_DIR}/msp430-elf-size")

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

enable_language(ASM)

#
# Compiler flags.
# See https://gcc.gnu.org/onlinedocs/gcc/MSP430-Options.html
# and https://www.ti.com/lit/ug/slau646f/slau646f.pdf
#

# Set MCU target
set(MCU_SPECIFIC_CFLAGS "-mmcu=msp430fr5994")
# Set type of hardware multiply supported by the target.
string(APPEND MCU_SPECIFIC_CFLAGS " -mhwmult=none")
# Use large-model addressing
string(APPEND MCU_SPECIFIC_CFLAGS " -mlarge")
# Toolchain header files
string(APPEND MCU_SPECIFIC_CFLAGS " -I${TOOLCHAIN_PREFIX}/msp430-elf/sys-include")
# string(APPEND MCU_SPECIFIC_CFLAGS " -T../bootloader_target.ld")

string(APPEND MCU_SPECIFIC_CFLAGS " -mcode-region=none")
string(APPEND MCU_SPECIFIC_CFLAGS " -mdata-region=none")
string(APPEND MCU_SPECIFIC_CFLAGS " -fdata-sections")
string(APPEND MCU_SPECIFIC_CFLAGS " -ffunction-sections")
string(APPEND MCU_SPECIFIC_CFLAGS " -nostdlib")
string(APPEND MCU_SPECIFIC_CFLAGS " -fno-builtin")
string(APPEND MCU_SPECIFIC_CFLAGS " -nostartfiles")
# string(APPEND MCU_SPECIFIC_CFLAGS " -mhwmult=none")

# string(APPEND MCU_SPECIFIC_CFLAGS " -D__MSP430__" "-D__MSP430FR5994__")
set(CMAKE_ASM_FLAGS "-Wa,-g,--gdwarf-2,--gdwarf-sections -Wall -mmcu=msp430fr5994 -mlarge -mcode-region=none -mdata-region=none -I${TOOLCHAIN_PREFIX}/msp430-elf/sys-include" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "${MCU_SPECIFIC_CFLAGS}" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -gdwarf-2 -ggdb -gstrict-dwarf" CACHE INTERNAL "c compiler flags debug")
set(CMAKE_C_FLAGS_RELEASE "-Os -flto -DNDEBUG" CACHE INTERNAL "c compiler flags release")

#
# Linker flags.
#
# This path contains default linker scripts
# set(MCU_SPECIFIC_LINKER_FLAGS "-L${TOOLCHAIN_PREFIX}/msp430-elf/sys-include -T/home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/bootloader_target.ld")
# set(MCU_SPECIFIC_LINKER_FLAGS "-Wl,--no-undefined -Wl,--gc-sections -Wl,-Map=bootloader.map -Wl,--undefined=__mspabi_mpyi -lmul_f5")
# set(CMAKE_EXE_LINKER_FLAGS "${MCU_SPECIFIC_LINKER_FLAGS} -Wl,-L${TOOLCHAIN_PREFIX}/msp430-elf/sys-include -Wl,-T/home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/bootloader_target.ld -Wl,-L/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/msp430-elf/lib/large/full-memory-range -Wl,-lc -Wl,-L/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/lib/gcc/msp430-elf/9.3.1/large/full-memory-range -Wl,-lgcc" CACHE STRING "")

set(LINKER_SPECIFIC_FLAGS "--no-undefined --gc-sections -Map=bootloader.map -L${TOOLCHAIN_PREFIX}/msp430-elf/sys-include -T/home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/bootloader_target.ld -L/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/msp430-elf/lib/large/full-memory-range -lc -L/home/bernabeu-a/Source/msp430-gcc-9.3.1.11_linux64/lib/gcc/msp430-elf/9.3.1/large/full-memory-range -lgcc")
set(CMAKE_C_LINK_EXECUTABLE
    "${CMAKE_LINKER} -o bootloader.out /home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/build/CMakeFiles/bootloader.out.dir/boot.S.o /home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/build/CMakeFiles/bootloader.out.dir/main.c.o /home/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bootloader/bootloader_target/build/CMakeFiles/bootloader.out.dir/lora/lora.c.o ${LINKER_SPECIFIC_FLAGS}")

    # set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(foo)

add_executable(bootloader.out
    boot.S
    main.c
    lora/lora.c
)