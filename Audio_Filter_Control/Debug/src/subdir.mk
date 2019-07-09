################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../src/patrick_flasher.asm 

C_SRCS += \
../src/dma_non_blocking.c 

CPP_SRCS += \
../src/audio.cpp \
../src/audio_processing.cpp 

SRC_OBJS += \
./src/audio.doj \
./src/audio_processing.doj \
./src/dma_non_blocking.doj \
./src/patrick_flasher.doj 

ASM_DEPS += \
./src/patrick_flasher.d 

C_DEPS += \
./src/dma_non_blocking.d 

CPP_DEPS += \
./src/audio.d \
./src/audio_processing.d 


# Each subdirectory must supply rules for building sources it contributes
src/audio.doj: ../src/audio.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="Audio_Filter_Control" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-40c0edc3f2b7aae49c4c2fc1dac80628.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/audio.d" -c++ -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/audio_processing.doj: ../src/audio_processing.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="Audio_Filter_Control" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-40c0edc3f2b7aae49c4c2fc1dac80628.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/audio_processing.d" -c++ -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/dma_non_blocking.doj: ../src/dma_non_blocking.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="Audio_Filter_Control" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-40c0edc3f2b7aae49c4c2fc1dac80628.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/dma_non_blocking.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/patrick_flasher.doj: ../src/patrick_flasher.asm
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin Assembler'
	easmblkfn -file-attr ProjectName="Audio_Filter_Control" -proc ADSP-BF706 -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-3c7c6f4efb5e56e8ef266216737ececf.txt -gnu-style-dependencies -MM -Mo "src/patrick_flasher.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


