################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../lnk_msp430fr5949.cmd 

CFG_SRCS += \
../main.cfg 

C_SRCS += \
../leds.c \
../main.c \
../radio.c 

OBJS += \
./leds.obj \
./main.obj \
./radio.obj 

C_DEPS += \
./leds.pp \
./main.pp \
./radio.pp 

GEN_MISC_DIRS += \
./configPkg/ 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_OPTS += \
./configPkg/compiler.opt 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

C_DEPS__QUOTED += \
"leds.pp" \
"main.pp" \
"radio.pp" 

OBJS__QUOTED += \
"leds.obj" \
"main.obj" \
"radio.obj" 

C_SRCS__QUOTED += \
"../leds.c" \
"../main.c" \
"../radio.c" 

GEN_CMDS__FLAG += \
-l"./configPkg/linker.cmd" 

GEN_OPTS__FLAG += \
--cmd_file="./configPkg/compiler.opt" 


