cmake_minimum_required(VERSION 3.0)

project(QuartzDependencies)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/SDL2/include)
	message("It seems like the Git Submodules have not been initialised and cloned, they will now be initialised and cloned as required.")
	execute_process(COMMAND git submodule update --init
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()

set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)

add_subdirectory(SDL2)
add_subdirectory(sol2)
add_subdirectory(lua)

# stop SDL trying to control our main function.
add_definitions(-DSDL_MAIN_HANDLED)

set_target_properties(SDL2main SDL2-static uninstall PROPERTIES FOLDER QuartzDependencies/sdl2)
