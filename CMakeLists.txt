cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project(DoomRPG LANGUAGES C)

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(FluidSynth)
if(NOT FluidSynth_FOUND)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth)
	set(FLUIDSYNTH_LIBRARIES ${FLUIDSYNTH_LIBRARIES})
	set(FLUIDSYNTH_INCLUDE_DIRS ${FLUIDSYNTH_INCLUDE_DIRS})
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/src")