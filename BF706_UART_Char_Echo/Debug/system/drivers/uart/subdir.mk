################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/Blackfin/lib/src/drivers/source/uart/adi_uart.c 

SRC_OBJS += \
./system/drivers/uart/adi_uart.doj 

C_DEPS += \
./system/drivers/uart/adi_uart.d 


# Each subdirectory must supply rules for building sources it contributes
system/drivers/uart/adi_uart.doj: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/Blackfin/lib/src/drivers/source/uart/adi_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="BF706_char_echo" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DADI_DEBUG -DNO_UTILITY_ROM_UART -DCORE0 @includes-937ae863d339a2c9811e3d97a5309eea.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -no-utility-rom -gnu-style-dependencies -MD -Mo "system/drivers/uart/adi_uart.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


