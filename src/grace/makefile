# path to use to find driver sources
DRV_SRCDIR = C:/ti/msp430/MSPWare_2_20_00_19/driverlib/driverlib/MSP430FR5xx_6xx
vpath %.c $(DRV_SRCDIR)

# optional compiler flags
CCOPTS ?= -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/glouthan/workspace_v6_0/qc12" --include_path="C:/Users/glouthan/workspace_v6_0/qc12/grlib" --include_path="C:/Users/glouthan/workspace_v6_0/qc12/LCD_Driver" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --advice:power=all --advice:hw_config=all -g --c99 --define=__MSP430FR5949__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal   -qq -pdsw225 

# required C preprocessor flags
#    compiler   headers: "C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include"
#    device     headers: "C:/ti/ccsv6/ccs_base/msp430/include"
#    Grace      headers: "C:/ti/grace_3_10_00_82/packages"
#    MSP430ware headers: "C:/ti/msp430/MSPWare_2_20_00_19/driverlib/driverlib/MSP430FR5xx_6xx"
#                      : "C:/ti/msp430/MSPWare_2_20_00_19/driverlib"
CPPFLAGS = -D__MSP430FR5949__ -I "C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" -I "C:/ti/grace_3_10_00_82/packages"  -I "C:/ti/ccsv6/ccs_base/msp430/include" -I "C:/ti/msp430/MSPWare_2_20_00_19/driverlib/driverlib/MSP430FR5xx_6xx" -I "C:/ti/msp430/MSPWare_2_20_00_19/driverlib"

# the compiler and archiver commands
CC = "C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" --abi=eabi -c --obj_directory=objs/  -vmspx --near_data=none --code_model=large --data_model=restricted
AR = "C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/ar430" rq
RM = cmd.exe /q /c del /f
MKDIR = mkdir

# create object sub-directory, if necessary
ifeq (,$(wildcard objs/))
    $(shell $(MKDIR) objs\)
endif

all: grace.lib

# always add power advice options to the Grace generated files 
objs/Grace_init.obj objs/CSL_init.obj objs/Timer1_A3_init.obj objs/PinMux_init.obj objs/CS_A_init.obj objs/EUSCI_A0_init.obj objs/EUSCI_A1_init.obj objs/EUSCI_B0_init.obj objs/PMM_init.obj objs/InterruptVectors_init.obj objs/System_init.obj objs/RTC_B_init.obj objs/WDT_A_init.obj: CCOPTS := $(CCOPTS) --advice:power=all

# supress power advice for driverlib by default; the user can override this by setting project options
 objs/adc12_b.obj objs/aes256.obj objs/comp_e.obj objs/crc.obj objs/crc32.obj objs/cs.obj objs/dma.obj objs/esi.obj objs/eusci_a_spi.obj objs/eusci_a_uart.obj objs/eusci_b_i2c.obj objs/eusci_b_spi.obj objs/framctl.obj objs/gpio.obj objs/lcd_c.obj objs/mpu.obj objs/mpy32.obj objs/pmm.obj objs/ram.obj objs/ref_a.obj objs/rtc_b.obj objs/rtc_c.obj objs/sfr.obj objs/sysctl.obj objs/timer_a.obj objs/timer_b.obj objs/tlv.obj objs/wdt_a.obj:  CCOPTS := --advice:power_severity=suppress $(CCOPTS)

# pattern rule to compile .c source to an object file
objs/%.obj: %.c makefile
	@echo cle430X $< ...
	$(CC) $(CCOPTS) $(CPPFLAGS) "$<"

# rule to combine all objects into the grace.lib library
grace.lib: objs/Grace_init.obj objs/CSL_init.obj objs/Timer1_A3_init.obj objs/PinMux_init.obj objs/CS_A_init.obj objs/EUSCI_A0_init.obj objs/EUSCI_A1_init.obj objs/EUSCI_B0_init.obj objs/PMM_init.obj objs/InterruptVectors_init.obj objs/System_init.obj objs/RTC_B_init.obj objs/WDT_A_init.obj objs/adc12_b.obj objs/aes256.obj objs/comp_e.obj objs/crc.obj objs/crc32.obj objs/cs.obj objs/dma.obj objs/esi.obj objs/eusci_a_spi.obj objs/eusci_a_uart.obj objs/eusci_b_i2c.obj objs/eusci_b_spi.obj objs/framctl.obj objs/gpio.obj objs/lcd_c.obj objs/mpu.obj objs/mpy32.obj objs/pmm.obj objs/ram.obj objs/ref_a.obj objs/rtc_b.obj objs/rtc_c.obj objs/sfr.obj objs/sysctl.obj objs/timer_a.obj objs/timer_b.obj objs/tlv.obj objs/wdt_a.obj
	@ echo are430X $@ $< ...
	$(AR) $@ $^

# clean removes all generated files
clean:
	-$(RM) grace.lib
	-$(RM) "objs\"*.obj

