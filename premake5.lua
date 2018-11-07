
project_name = 'pcb-printer'
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