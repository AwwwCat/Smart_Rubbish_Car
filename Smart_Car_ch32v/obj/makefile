################################################################################
# MRS Version: {"version":"1.8.4","date":"2023/02/015"}
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
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

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Smart_Car.hex \

SECONDARY_LIST += \
Smart_Car.lst \

SECONDARY_SIZE += \
Smart_Car.siz \


# 所有目标
all: Smart_Car.elf secondary-outputs

# 工具调用
Smart_Car.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "C:\Users\lenovo\Desktop\MRS\Smart_Car\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"Smart_Car.map" --specs=nano.specs --specs=nosys.specs -o "Smart_Car.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@	@
Smart_Car.hex: Smart_Car.elf
	@	riscv-none-embed-objcopy -O ihex "Smart_Car.elf"  "Smart_Car.hex"
	@	@
Smart_Car.lst: Smart_Car.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "Smart_Car.elf" > "Smart_Car.lst"
	@	@
Smart_Car.siz: Smart_Car.elf
	@	riscv-none-embed-size --format=berkeley "Smart_Car.elf"
	@	@
# 其他目标
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) Smart_Car.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
