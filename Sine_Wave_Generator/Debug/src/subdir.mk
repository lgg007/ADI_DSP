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
	ccblkfn -c -file-attr ProjectName="Sine_Wave_Generator" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-ab86b2bdb2158dbb497c9f47104f429b.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/audio.d" -c++ -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/audio_processing.doj: ../src/audio_processing.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="Sine_Wave_Generator" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-ab86b2bdb2158dbb497c9f47104f429b.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/audio_processing.d" -c++ -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/dma_non_blocking.doj: ../src/dma_non_blocking.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin C/C++ Compiler'
	ccblkfn -c -file-attr ProjectName="Sine_Wave_Generator" -proc ADSP-BF706 -flags-compiler --no_wrap_diagnostics -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-ab86b2bdb2158dbb497c9f47104f429b.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -decls-strong -cplbs -gnu-style-dependencies -MD -Mo "src/dma_non_blocking.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/patrick_flasher.doj: ../src/patrick_flasher.asm
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore Blackfin Assembler'
	easmblkfn -file-attr ProjectName="Sine_Wave_Generator" -proc ADSP-BF706 -si-revision 1.1 -g -D_DEBUG -DCORE0 @includes-a98da370a91eb5b393dcdae8a56d894d.txt -gnu-style-dependencies -MM -Mo "src/patrick_flasher.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


