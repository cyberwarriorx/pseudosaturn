CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(concat)

set(concat_SOURCES
	main.c)

add_executable(${PROJECT_NAME} ${concat_SOURCES})
