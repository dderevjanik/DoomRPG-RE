cmake_minimum_required(VERSION 3.25)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project(DoomRPG LANGUAGES C)

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

# Emscripten (WebAssembly) build configuration
if(EMSCRIPTEN)
	message(STATUS "Building for Emscripten (WebAssembly)")

	# Use SDL2 from Emscripten ports
	set(USE_FLAGS "-sUSE_SDL=2 -sUSE_ZLIB=1")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")

	# Memory settings
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sINITIAL_MEMORY=67108864")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sALLOW_MEMORY_GROWTH=1")

	# Game assets (DoomRPG.zip) are NOT bundled - user uploads their own copy
	# which is stored in IndexedDB for subsequent visits

	# Use custom HTML shell template
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html")

	# Generate HTML file
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# Export main function for Emscripten
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sEXPORTED_FUNCTIONS=_main")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,FS")

	# Force filesystem support (needed for writing ZIP to virtual FS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sFORCE_FILESYSTEM=1")

	# Modularize to prevent auto-execution - we call the module factory after ZIP is loaded
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMODULARIZE=1")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sEXPORT_NAME=createDoomRPG")

	# Audio is disabled for web builds - no SDL_mixer or FluidSynth needed
	add_definitions(-D__EMSCRIPTEN__)

else()
	# Native build - find packages normally
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
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/src")