cmake_minimum_required(VERSION 3.22)
# Set this value...
set(VCPKG_BASE_DIR "C:/vcpkg")
# Or this value
set(CMAKE_CONFIG_FILE_PATH "")

set(CMAKE_CONFIGURATION_TYPES "$CACHE{CMAKE_BUILD_TYPE}")

if (VCPKG_BASE_DIR)
	set(THE_TOOLCHAIN_PATH "${VCPKG_BASE_DIR}/scripts/buildsystems/vcpkg.cmake")

	set(CMAKE_TOOLCHAIN_FILE "${THE_TOOLCHAIN_PATH}" CACHE FILEPATH "The toolchain file.")
	set(ENV{CMAKE_TOOLCHAIN_FILE} "${THE_TOOLCHAIN_PATH}")
	string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" OS_NAME)
	set(VCPKG_INSTALLED_DIR_NEW "${VCPKG_BASE_DIR}/installed/x64-${OS_NAME}")
elseif (CMAKE_CONFIG_FILE_PATH)
	set(DiscordCoreAPI_DIR ${CMAKE_CONFIG_FILE_PATH})
else()
	message(FATAL_ERROR "Please, set the VCPKG base directory, or the location of the DiscordCoreAPIConfig.cmake file!")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(generate_product_version)

generate_product_version(
   VERSION_FILES_OUTPUT_VARIABLE
   NAME "MBot-Janny-Cpp"
   ICON "${CMAKE_SOURCE_DIR}/Chris 03.ico"
   ORIGINAL_FILENAME "MBot-Janny-Cpp.exe"
   VERSION_MAJOR 1
   VERSION_MINOR 0
   VERSION_PATCH 0
   VERSION_REVISION 0
)

project(MBot-Janny LANGUAGES CXX)

add_executable (
	MBot-Janny
	"main.cpp"
	"${VERSION_FILES_OUTPUT_VARIABLE}"
)

find_package(DiscordCoreAPI CONFIG REQUIRED)

target_link_libraries(
	MBot-Janny PUBLIC
	DiscordCoreAPI::DiscordCoreAPI
	"${VCPKG_INSTALLED_DIR_NEW}/lib/mongoc-1.0.lib"
	"${VCPKG_INSTALLED_DIR_NEW}/lib/mongocxx.lib"
	"${VCPKG_INSTALLED_DIR_NEW}/lib/bsoncxx.lib"
)

set_target_properties(
	MBot-Janny PROPERTIES 
	OUTPUT_NAME "MBot-Janny-Cpp"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
)

if (WIN32)
	add_custom_command(TARGET MBot-Janny POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:MBot-Janny> $<TARGET_FILE_DIR:MBot-Janny>
	  COMMAND_EXPAND_LISTS
	)
endif()
