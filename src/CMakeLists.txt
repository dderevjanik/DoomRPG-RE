file(GLOB_RECURSE HEADER_FILES
	"*.hpp"
	"*.h")

file(GLOB_RECURSE SOURCE_FILES
	"*.cpp"
	"*.c")

file(GLOB_RECURSE RESOURCES_FILES
	"*.rc"
	"*.ico")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Resources Files" FILES ${RESOURCES_FILES})

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCES_FILES})
if(TARGET FluidSynth::libfluidsynth)
	target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::mixer ZLIB::ZLIB FluidSynth::libfluidsynth)
elseif(FLUIDSYNTH_LIBRARIES)
	target_include_directories(${PROJECT_NAME} PRIVATE ${FLUIDSYNTH_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::mixer ZLIB::ZLIB ${FLUIDSYNTH_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::mixer ZLIB::ZLIB)
endif()