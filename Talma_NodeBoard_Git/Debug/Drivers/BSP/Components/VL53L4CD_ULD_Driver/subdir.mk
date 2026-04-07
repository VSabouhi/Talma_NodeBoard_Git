################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.c \
../Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.c 

OBJS += \
./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.o \
./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.o 

C_DEPS += \
./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.d \
./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/VL53L4CD_ULD_Driver/%.o Drivers/BSP/Components/VL53L4CD_ULD_Driver/%.su Drivers/BSP/Components/VL53L4CD_ULD_Driver/%.cyclo: ../Drivers/BSP/Components/VL53L4CD_ULD_Driver/%.c Drivers/BSP/Components/VL53L4CD_ULD_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I"D:/gKteso/TALMA/Software/ARM/STM32767/Talma_NodeBoard_Git/Talma_NodeBoard_Git/Drivers/BSP/Components" -I"D:/gKteso/TALMA/Software/ARM/STM32767/Talma_NodeBoard_Git/Talma_NodeBoard_Git/Drivers/BSP/Components/VL53L4CD_ULD_Driver" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-VL53L4CD_ULD_Driver

clean-Drivers-2f-BSP-2f-Components-2f-VL53L4CD_ULD_Driver:
	-$(RM) ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.cyclo ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.d ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.o ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_api.su ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.cyclo ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.d ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.o ./Drivers/BSP/Components/VL53L4CD_ULD_Driver/VL53L4CD_calibration.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-VL53L4CD_ULD_Driver

