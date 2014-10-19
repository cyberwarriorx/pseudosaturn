project(PseudoSaturn)

enable_language(ASM)
set(PseudoSaturn_SOURCES
	shared/crt0.s
	cdload.c
	jhloader.c
	main.c)

if (PS_WANT_CHEATS)
    set(PseudoSaturn_SOURCES ${PseudoSaturn_SOURCES}
        cheat.c
        codes.c
        mastercode.s)
endif()

set(CMAKE_C_FLAGS "-O2 -Wall -m2")
set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--script,${CMAKE_SOURCE_DIR}/src/shared/bart.lk")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_definitions(-DPSEUDOSATURN_VERSION=\"${PSEUDOSATURN_VERSION}\")

set_source_files_properties(cdload.c PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cdload/cdload.bin)

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

add_subdirectory(installer)

install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.BIN DESTINATION ".")
install(FILES ${CMAKE_SOURCE_DIR}/CHANGELOG DESTINATION ".")
install(FILES ${CMAKE_SOURCE_DIR}/README.TXT DESTINATION ".")

