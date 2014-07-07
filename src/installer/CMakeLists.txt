project(installer)

enable_language(ASM)
set(installer_SOURCES
	crt0.s
	main.c
    ps_rom.c)

set(CMAKE_C_FLAGS "-O2 -Wall -m2")
set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--script,${CMAKE_SOURCE_DIR}/src/installer/bart.lk")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

find_program(CMAKE_MKISOFS
    NAMES mkisofs)

add_executable(${PROJECT_NAME} ${installer_SOURCES})

target_link_libraries(${PROJECT_NAME} ${IAPETUS_LIB} )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${PROJECT_NAME}
	COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMENT "Generating bin file ${PROJECT_NAME}.bin" )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${PROJECT_NAME}.bin
	COMMAND ${CMAKE_MKISOFS} 
	ARGS -G ${CMAKE_SOURCE_DIR}/src/installer/ip.bin -sysid "SEGA SEGASATURN" -volid "PSEUDOSATURN INSTALLER" -volset "PSEUDOSATURN INSTALLER" -publisher "Cyber Warrior X" -preparer "Cyber Warrior X" -appid "PSEUDOSATURN INSTALLER" -abstract "abs.txt" -copyright "cpy.txt" -biblio "bib.txt" -full-iso9660-filenames -o ${PROJECT_NAME}.iso ${PROJECT_NAME}.bin ${CMAKE_SOURCE_DIR}/src/installer/padding
	COMMENT "Generating disc image for ${PROJECT_NAME}")

#install(TARGETS ${PROJECT_NAME}.iso)

