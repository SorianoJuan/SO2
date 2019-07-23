################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RecTrace/trcKernelPort.c \
../RecTrace/trcSnapshotRecorder.c \
../RecTrace/trcStreamingPort.c \
../RecTrace/trcStreamingRecorder.c 

OBJS += \
./RecTrace/trcKernelPort.o \
./RecTrace/trcSnapshotRecorder.o \
./RecTrace/trcStreamingPort.o \
./RecTrace/trcStreamingRecorder.o 

C_DEPS += \
./RecTrace/trcKernelPort.d \
./RecTrace/trcSnapshotRecorder.d \
./RecTrace/trcStreamingPort.d \
./RecTrace/trcStreamingRecorder.d 


# Each subdirectory must supply rules for building sources it contributes
RecTrace/%.o: ../RecTrace/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSIS_CORE_LPC17xx -D__LPC17XX__ -D__REDLIB__ -I"/home/torce/Facultad/SO2/so2/TP4/CMSIS_CORE_LPC17xx/inc" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/FreeRTOS" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/FreeRTOS/include" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/FreeRTOS/portable" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/src" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/RecTrace/config" -I"/home/torce/Facultad/SO2/so2/TP4/tp4_Soriano/RecTrace/include" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


