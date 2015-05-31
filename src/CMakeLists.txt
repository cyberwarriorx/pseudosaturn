project(PseudoSaturn)

enable_language(ASM)

set(PseudoSaturn_SOURCES
	crt0.s
	jhloader.c
	main.c
    img.c
    logo.c)

if (PS_WANT_DEBUG)
	set_source_files_properties(crt0.s PROPERTIES COMPILE_FLAGS "-Wa,--defsym,ENABLE_DEBUG=1")
endif()

if (PS_WANT_CHEATS)
    set(PseudoSaturn_SOURCES ${PseudoSaturn_SOURCES}
        cheat.c
        codes.c
        mastercode.s)
endif()

set(CMAKE_C_FLAGS "-O2 -Wall -m2")
if (PS_WANT_DEBUG)
    set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--script,${CMAKE_SOURCE_DIR}/src/debug.lk")
else()
    set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--script,${CMAKE_SOURCE_DIR}/src/bart.lk")
endif()
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_definitions(-DPSEUDOSATURN_VERSION=\"${PSEUDOSATURN_VERSION}\")
add_definitions(-DREENTRANT_SYSCALLS_PROVIDED)
add_definitions(-DMISSING_SYSCALL_NAMES)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/codes.c
	DEPENDS codes.bin
	COMMAND ${CMAKE_BIN2C} 
	ARGS ${CMAKE_SOURCE_DIR}/src/codes.bin ${CMAKE_SOURCE_DIR}/src/codes.c cheatlist
    COMMENT "Generating source file from codes.bin" )

add_executable(${PROJECT_NAME} ${PseudoSaturn_SOURCES})

target_link_libraries(${PROJECT_NAME} ${PseudoSaturn_LIBRARIES} )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${PROJECT_NAME}
	COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${PROJECT_NAME}.elf ${CMAKE_BINARY_DIR}/PS.BIN
    COMMENT "Generating bin file PS.bin" )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${CMAKE_BINARY_DIR}/PS.BIN
	COMMAND ${CMAKE_CONCAT} ${CMAKE_SOURCE_DIR}/src/IP.BIN ${CMAKE_BINARY_DIR}/PS.BIN ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.BIN
	COMMENT "Generating bin file PseudoSaturn.bin" )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.BIN
	COMMAND ${CMAKE_BIN2C} 
	ARGS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.BIN ${CMAKE_SOURCE_DIR}/src/installer/ps_rom.c ps_rom
	COMMENT "Generating source file out of PS.bin")

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/logo.c
	DEPENDS ${CMAKE_SOURCE_DIR}/src/pseudo_02_8.pcx
	COMMAND ${CMAKE_BIN2C} 
	ARGS ${CMAKE_SOURCE_DIR}/src/pseudo_02_8.pcx ${CMAKE_SOURCE_DIR}/src/logo.c logo
	COMMENT "Generating source file out of pseudo_02_8.pcx")


add_subdirectory(installer)

install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.BIN DESTINATION ".")
install(FILES ${CMAKE_SOURCE_DIR}/CHANGELOG DESTINATION ".")
install(FILES ${CMAKE_SOURCE_DIR}/README.TXT DESTINATION ".")

