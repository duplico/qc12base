################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
leds.obj: ../leds.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/George/workspace_v6_0/qc12/grlib" --include_path="C:/Users/George/workspace_v6_0/qc12/LCD_Driver" --include_path="C:/Users/George/workspace_v6_0/qc12/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --advice:power=all --advice:hw_config=all -g --c99 --define=__MSP430FR5949__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="leds.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/George/workspace_v6_0/qc12/grlib" --include_path="C:/Users/George/workspace_v6_0/qc12/LCD_Driver" --include_path="C:/Users/George/workspace_v6_0/qc12/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --advice:power=all --advice:hw_config=all -g --c99 --define=__MSP430FR5949__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: ../main.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_31_00_24_core/xs" --xdcpath="c:/ti/grace_3_00_03_66/packages;C:/ti/ccsv6/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.msp430.elf.MSP430X -p ti.platforms.msp430:MSP430FR5949 -r debug -c "C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4" -Dxdc.cfg.tsort.policy=fast -Dxdc.cfg.gen.metadataFiles=false -Dxdc.cfg.SourceDir.verbose=7 --compileOptions "-vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path=\"C:/ti/ccsv6/ccs_base/msp430/include\" --include_path=\"C:/Users/George/workspace_v6_0/qc12/grlib\" --include_path=\"C:/Users/George/workspace_v6_0/qc12/LCD_Driver\" --include_path=\"C:/Users/George/workspace_v6_0/qc12/driverlib/MSP430FR5xx_6xx\" --include_path=\"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include\" --advice:power=all --advice:hw_config=all -g --c99 --define=__MSP430FR5949__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/compiler.opt: | configPkg/linker.cmd
configPkg/: | configPkg/linker.cmd

radio.obj: ../radio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/George/workspace_v6_0/qc12/grlib" --include_path="C:/Users/George/workspace_v6_0/qc12/LCD_Driver" --include_path="C:/Users/George/workspace_v6_0/qc12/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --advice:power=all --advice:hw_config=all -g --c99 --define=__MSP430FR5949__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="radio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


