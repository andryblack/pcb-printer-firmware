
project_name = 'pcb-printer'

newoption {
	trigger = "com",
	value = "string",
	description = "communication port",
	default = 'serial',
	allowed = { 
		{'serial','Serial port'},
		{'usb','USB CDC'},
	}
}

include 'main'

stm32_solution(project_name)
stm32_project(project_name)


includedirs{ 'src' }
includedirs{ 'platform' }


files {
	'src/**',
	'platform/gpio.*',
	'platform/dma.*',
	'platform/timer.*',
	'platform/uart.*',
}


if _OPTIONS['com'] == 'usb' then
	local usb_desc = require 'usb_descriptor'
	local descr_file = usb_desc.descriptor{
		VID = 1155,
		PID = 22336,
		manufacturer = 'DIY',
		product = 'STM32 PCB Printer',
		serialnumber = '00000000121C',
		classes = {'cdc'},
		configs = {
			{
				name = 'cdc_class',
				interfaces = {
					{
						name = 'cdc_control',
						class = 'CDC',
						subclass = 'ABSTRACT_CONTROL',
						protocol = 'AT_COMMANDS',
						master = 0,
						data_interface = 1,
						slaves = { 1 },
						endpoints = {
							{
								idx = 1,
								out = true,
								size = 8,
								attr = 'USBD_EP_TYPE_INTR',
								interval = '0x10'
							}
						}
					},
					{
						name = 'cdc_data',
						class = 'DATA',
						subclass = 'UNDEFINED',
						protocol = 'UNDEFINED',
						endpoints = {
							{
								idx = 2,
								out = false,
								size = 64,
								attr = 'USBD_EP_TYPE_BULK',
							},
							{
								idx = 2,
								out = true,
								size = 64,
								attr = 'USBD_EP_TYPE_BULK',
							}
						}
					}
				}
			}
		}
	}

	defines {
		'USE_USB_COM',
	}
	files {
		descr_file,
		'platform/usb.*',
		'platform/usb/core/**',
		'platform/usb/cdc/**'
	}
end