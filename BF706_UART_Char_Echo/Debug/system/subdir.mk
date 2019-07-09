################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/adi_initialize.c 

SRC_OBJS += \
./system/adi_initialize.doj 

C_DEPS += \
./system/adi_initialize.d 


# Each subdirectory must supply rules for building sources it contributes
system/adi_initialize.doj: ../system/adi_initialize.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="BF706_char_echo" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DADI_DEBUG -DNO_UTILITY_ROM_UART -DCORE0 @includes-937ae863d339a2c9811e3d97a5309eea.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -no-utility-rom -gnu-style-dependencies -MD -Mo "system/adi_initialize.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


