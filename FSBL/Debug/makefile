################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Utilities/lcd/subdir.mk
-include Model/STM32N6570-DK/subdir.mk
-include Middlewares/usbx/UX\ Device\ CoreStack/subdir.mk
-include Middlewares/usbx/UX\ Device\ Controllers/subdir.mk
-include Middlewares/usbx/UX\ Device\ Class\ CDC\ ACM/subdir.mk
-include Middlewares/usbx/Core\ System/subdir.mk
-include Middlewares/threadx/Core/subdir.mk
-include Middlewares/stm32-vision-models-postprocessing/lib_vision_models_pp/Src/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/vd6g/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/vd55g1/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/vd1943/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/ov5640/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/imx335/subdir.mk
-include Middlewares/stm32-mw-camera/sensors/subdir.mk
-include Middlewares/stm32-mw-camera/ISP_Library/isp/Src/subdir.mk
-include Middlewares/stm32-mw-camera/subdir.mk
-include Middlewares/stedgeai-lib/Npu/ll_aton/subdir.mk
-include Middlewares/stedgeai-lib/Npu/Devices/STM32N6xx/subdir.mk
-include Middlewares/screenl/ili9341/subdir.mk
-include Middlewares/screenl/Src/subdir.mk
-include Middlewares/netxduo/tsn/src/subdir.mk
-include Middlewares/netxduo/common/src/subdir.mk
-include Middlewares/netxduo/common/drivers/ethernet/rtl8211/subdir.mk
-include Middlewares/netxduo/common/drivers/ethernet/subdir.mk
-include Middlewares/netxduo/addons/web/subdir.mk
-include Middlewares/netxduo/addons/dhcp/subdir.mk
-include Middlewares/filex/common/src/subdir.mk
-include Middlewares/filex/common/drivers/subdir.mk
-include Middlewares/ai-postprocessing-wrapper/subdir.mk
-include Middlewares/STM32_USBPD_Library/subdir.mk
-include Drivers/STM32N6xx_HAL_Driver/subdir.mk
-include Drivers/BSP/STM32N6570-DK/subdir.mk
-include Drivers/BSP/Components/subdir.mk
-include Drivers/subdir.mk
-include Application/USBX/App/subdir.mk
-include Application/USBPD/Target/subdir.mk
-include Application/USBPD/App/subdir.mk
-include Application/Startup/subdir.mk
-include Application/RTOS/subdir.mk
-include Application/NetXDuo/Target/subdir.mk
-include Application/NetXDuo/App/subdir.mk
-include Application/LCD/subdir.mk
-include Application/Hardware/subdir.mk
-include Application/FileX/Target/subdir.mk
-include Application/FileX/App/subdir.mk
-include Application/Core/Src/subdir.mk
-include Application/Camera/subdir.mk
-include Application/AI/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.elf \

MAP_FILES += \
STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.list \

OBJCOPY_BIN += \
STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.bin \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.elf secondary-outputs

# Tool invocations
STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.elf STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.map: $(OBJS) $(USER_OBJS) /home/ck/Desktop/Workspace/STM32CubeIDE\ Workspace/STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet/FSBL/STM32N657X0HXQ_AXISRAM2_fsbl.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m55 -T"/home/ck/Desktop/Workspace/STM32CubeIDE Workspace/STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet/FSBL/STM32N657X0HXQ_AXISRAM2_fsbl.ld" --specs=nosys.specs -Wl,-Map="STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.map" -Wl,--gc-sections -static -Wl,--cmse-implib -Wl,--out-implib=./secure_nsclib.o -L../../Middlewares/ST/STM32_USBPD_Library/Core/lib --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.list"
	@echo 'Finished building: $@'
	@echo ' '

STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.bin STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.elf STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.list STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.map default.size.stdout
	-@echo ' '

post-build:
	cd "/home/ck/Desktop/Workspace/STM32CubeIDE Workspace/STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet/FSBL/Debug"
	echo y | /home/ck/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_SigningTool_CLI -bin "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL.bin" -nk -of 0x80000000 -t fsbl -o "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL-Trusted.bin" -hv 2.3 -dump "STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet-FSBL-Trusted.bin" -align
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
