CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(bin2c)

set(bin2c_SOURCES
	main.c)

add_executable(${PROJECT_NAME} ${bin2c_SOURCES})
