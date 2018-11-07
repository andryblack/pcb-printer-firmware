## PCB printer firmware

Firmware source code for PCB laser printer.

## Build

building depend on premake5(https://premake.github.io)

place config.mk with variable TOOLCHAINPATH, for example

```Makefile
TOOLCHAINPATH=$(HOME)/gcc-arm-none-eabi-6-2017-q1-update
```

build project and firmwre

```bash
$ make project
$ make release
```

firmware produced at bin folder

flashing via openocd
```bash
$ make flash
```

## License

This source code is available to anybody free of charge, under the terms of MIT License (see LICENSE).

For CMSIS licensing see source files.