
include config.mk

.PHONY: project clean flash distr

FIRMWARE=bin/pcb-printer.elf
GDB=$(TOOLCHAINPATH)/bin/arm-none-eabi-gdb
COMVARIANT?=serial

all : build-firmware

build/Makefile : project

project:
		premake5 --scripts=premake --chip=STM32F103C8T6 --toolchain_path=${TOOLCHAINPATH} --com=${COMVARIANT} arm_make 
	
		
	
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

flash-bmp:
		$(GDB) -nx --batch  \
			-ex "target extended-remote $(BMP_PORT)" \
			-ex "set remotetimeout 4000" \
			-ex "monitor version" \
			-ex "monitor swdp_scan" \
			-ex "attach 1" \
			-ex "load" \
			-ex 'compare-sections' \
			-ex 'kill' \
			$(FIRMWARE)

debug:
		$(GDB) $(FIRMWARE) \
			--eval-command="target extended-remote :3333" \
			--eval-command="set remotetimeout 2000" \
			--eval-command="monitor arm semihosting enable" \
			--eval-command="monitor reset" \
			--eval-command="load" \

distr: firmware.tar.gz


firmware.tar.gz: Makefile premake5.lua system src premake platform CMSIS openocd.cfg flash.cfg 
		tar czf $@ $^

# \
			