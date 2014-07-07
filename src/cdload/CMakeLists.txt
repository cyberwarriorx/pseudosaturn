project(cdload)

enable_language(ASM)
set(cdload_SOURCES
	crt0.s
	main.c)

set(CMAKE_C_FLAGS "-O2 -Wall -m2")
set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--script,${CMAKE_SOURCE_DIR}/src/cdload/bart.lk")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_executable(${PROJECT_NAME} ${cdload_SOURCES})

target_link_libraries(${PROJECT_NAME} ${IAPETUS_LIB} )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${PROJECT_NAME}
	COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMENT "Generating bin file ${PROJECT_NAME}.bin" )

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	DEPENDS ${PROJECT_NAME}.bin
	COMMAND ${CMAKE_BIN2C} 
	ARGS ${PROJECT_NAME}.bin ${CMAKE_SOURCE_DIR}/../cdload.c cdload
	COMMENT "Generating source file out of ${PROJECT_NAME}")
