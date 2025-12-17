################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle" --include_path="C:/ti/c2000/C2000Ware_5_05_00_00" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --define=DEBUG --define=RAM --define=generic_ram_lnk --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle/CPU1_RAM/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-97286625: ../gpio_ex2_toggle.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.23.2/sysconfig_cli.bat" --script "C:/Users/67981/workspace_v12/gpio_ex2_toggle/gpio_ex2_toggle.syscfg" -o "syscfg" -s "C:/ti/c2000/C2000Ware_5_05_00_00/.metadata/sdk.json" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-97286625 ../gpio_ex2_toggle.syscfg
syscfg/board.h: build-97286625
syscfg/board.cmd.genlibs: build-97286625
syscfg/board.opt: build-97286625
syscfg/board.json: build-97286625
syscfg/pinmux.csv: build-97286625
syscfg/device.c: build-97286625
syscfg/device.h: build-97286625
syscfg/device_cmd.cmd: build-97286625
syscfg/device_cmd.c: build-97286625
syscfg/device_cmd.h: build-97286625
syscfg/device_cmd.opt: build-97286625
syscfg/device_cmd.cmd.genlibs: build-97286625
syscfg/c2000ware_libraries.cmd.genlibs: build-97286625
syscfg/c2000ware_libraries.opt: build-97286625
syscfg/c2000ware_libraries.c: build-97286625
syscfg/c2000ware_libraries.h: build-97286625
syscfg/clocktree.h: build-97286625
syscfg: build-97286625

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle" --include_path="C:/ti/c2000/C2000Ware_5_05_00_00" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --define=DEBUG --define=RAM --define=generic_ram_lnk --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle/CPU1_RAM/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

f28003x_codestartbranch.obj: C:/ti/c2000/C2000Ware_5_05_00_00/device_support/f28003x/common/source/f28003x_codestartbranch.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle" --include_path="C:/ti/c2000/C2000Ware_5_05_00_00" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --define=DEBUG --define=RAM --define=generic_ram_lnk --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/67981/workspace_v12/gpio_ex2_toggle/CPU1_RAM/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


