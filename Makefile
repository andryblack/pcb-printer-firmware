
include config.mk

.PHONY: project clean flash distr

FIRMWARE=bin/pcb-printer.elf

all : build-firmware

build/Makefile : project

project:
		premake5 --scripts=premake --chip=STM32F103C8T6 --toolchain_path=${TOOLCHAINPATH} arm_make 
	
		
	
build-firmware:
		PATH=$(TOOLCHAINPATH)/bin:${PATH} $(MAKE) -C build verbose=1

release:
		PATH=$(TOOLCHAINPATH)/bin:${PATH} $(MAKE) -C build verbose=1 config=release

clean:
		$(MAKE) -C build clean
		$(MAKE) -C build clean config=release
		
distclean:
		rm -rf build bin


flash:
		openocd  -f flash.cfg 


debug:
		$(TOOLCHAINPATH)/bin/arm-none-eabi-gdb $(FIRMWARE) \
			--eval-command="target extended-remote :3333" \
			--eval-command="set remotetimeout 2000" \
			--eval-command="monitor arm semihosting enable" \
			--eval-command="monitor reset" \
			--eval-command="load" \

distr: firmware.tar.gz


firmware.tar.gz: Makefile premake5.lua system src premake platform CMSIS openocd.cfg flash.cfg 
		tar czf $@ $^

# \
			