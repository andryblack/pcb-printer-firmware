
local M = {}

local flash_size = {
	['4'] = 16,
	['6'] = 32,
	['8'] = 64,
	['B'] = 128,
	['C'] = 256,
	['D'] = 384,
	['E'] = 512,
	['F'] = 768,
	['G'] = 1024,
}

local chip_type = {
	['100'] = {
		['4'] = {4,'B'},
		['6'] = {4,'B'},
		['8'] = {8,'B'},
		['B'] = {8,'B'},
		['C'] = {24,'E'},
		['D'] = {32,'E'},
		['E'] = {32,'E'},
	},
	['101'] = {
		['4'] = {4,'6'},
		['6'] = {6,'6'},
		['8'] = {10,'B'},
		['B'] = {16,'B'},
		['C'] = {32,'E'},
		['D'] = {48,'E'},
		['E'] = {48,'E'},
		['F'] = {80,'G'},
		['G'] = {80,'G'},
 	},
 	['102'] = {
 		['4'] = {4,'6'},
 		['6'] = {6,'6'},
 		['8'] = {10,'B'},
 		['B'] = {16,'B'},
 	},
 	['103'] = {
 		['4'] = {6, '6'},
 		['6'] = {10,'6'},
 		['8'] = {20,'B'},
 		['B'] = {20,'B'},
 		['C'] = {48,'E'},
 		['D'] = {54,'E'},
 		['E'] = {54,'E'},
 		['F'] = {96,'G'},
 		['G'] = {96,'G'},
 	},
 	['105'] = {
 		['8'] = {64,'C'},
 		['B'] = {64,'C'},
 		['C'] = {64,'C'},
 	},
 	['107'] = {
 		['B'] = {64,'C'},
 		['C'] = {64,'C'},
 	}
}

local f1_flags = {
	c = {
		'-mthumb',
		'-mcpu=cortex-m3',
		'-Wall',
		'-std=gnu99',
		'-ffunction-sections',
		'-fdata-sections',
		'-fomit-frame-pointer',
		'-mabi=aapcs',
		'-fno-unroll-loops',
		'-fno-pic'
	},
	cxx = {
		'-mthumb',
		'-mcpu=cortex-m3',
		'-Wall',
		'-std=c++11',
		'-ffunction-sections',
		'-fdata-sections',
		'-fomit-frame-pointer',
		'-mabi=aapcs',
		'-fno-unroll-loops',
		'-fno-pic',
		'-finline-small-functions',
		'-finline-functions-called-once',
		'-fdelete-null-pointer-checks',
	},
	asm = {
		'-mthumb',
		'-mcpu=cortex-m3',
		'-x','assembler-with-cpp'
	},
	ld = {
		'-Wl,--gc-sections',
		'-mthumb',
		'-mcpu=cortex-m3',
		'-mabi=aapcs',
		'-fno-pic',
		'-Wl,--print-memory-usage',
		'-Wl,--relax',
	},
}
local chip_series = {
	['100'] = {'F1xx',f1_flags},
	['101'] = {'F1xx',f1_flags},
	['103'] = {'F1xx',f1_flags},
	['105'] = {'F1xx',f1_flags},
	['107'] = {'F1xx',f1_flags},
}


function M.parse( chip )

	local chip_f,chip_o = string.match(chip,'^[sS][tT][mM]32[fF](10[012357]).([468BCDEFG]).+$')
	assert(chip_f,'invalid chip')
	local chip_i = assert((chip_type[chip_f] or {})[chip_o],'uncnown chip ' .. chip)

	local res = {
		flash = assert(flash_size[chip_o],'unknown flash size code "' .. chip_o .. '"'),
		ram = chip_i[1],
		defs = {'-DSTM32F1','-DSTM32F' .. chip_f .. 'x' .. chip_i[2]},
		flags = chip_series[chip_f][2],
		f = chip_series[chip_f][1],
		low_name = string.lower('f'..chip_f..'x'..chip_i[2]),
	}

	res.ld_config = {
			STM32_FLASH_ORIGIN = '0x08000000',
			STM32_RAM_ORIGIN = '0x20000000',
			STM32_MIN_STACK_SIZE = '0x400',
			STM32_MIN_HEAP_SIZE = '0',
			STM32_CCRAM_ORIGIN = '0x10000000',
			STM32_CCRAM_SIZE = '64K',
			STM32_FLASH_SIZE = tostring(res.flash) .. 'K',
			STM32_RAM_SIZE = tostring(res.ram) .. 'K',
			STM32_CCRAM_DEF = '',
			STM32_CCRAM_SECTION = '',
		}

	return res
end

return M