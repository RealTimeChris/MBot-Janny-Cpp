cmake_minimum_required(VERSION 3.22)
# Set this value...
set(VCPKG_ROOT "C:/vcpkg")
# Or this value
set(CMAKE_CONFIG_FILE_PATH "")

set(CMAKE_CONFIGURATION_TYPES "$CACHE{CMAKE_BUILD_TYPE}")

if (VCPKG_ROOT)
	set(VCPKG_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
	if(NOT CMAKE_TOOLCHAIN_FILE AND VCPKG_TOOLCHAIN_FILE)
		set(CMAKE_TOOLCHAIN_FILE "${VCPKG_TOOLCHAIN_FILE}" CACHE FILEPATH "CMake toolchain file.")
		message(STATUS "Vcpkg toolchain found: $CACHE{CMAKE_TOOLCHAIN_FILE}")
	endif()
	string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" OS_NAME)
	set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed/x64-${OS_NAME}")
elseif (CMAKE_CONFIG_FILE_PATH)
	set(DiscordCoreAPI_DIR "${CMAKE_CONFIG_FILE_PATH}")
else()
	message(FATAL_ERROR "Please, set the VCPKG base directory, or the location of the DiscordCoreAPIConfig.cmake file!")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(GenerateProductVersion)

generate_product_version(
   PRODUCT_INFO
   NAME "MBot-Janny-Cpp"
   ICON "${CMAKE_SOURCE_DIR}/Chris 03.ico"
   ORIGINAL_FILENAME "MBot-Janny-Cpp.exe"
   VERSION_MAJOR "1"
   VERSION_MINOR "0"
   VERSION_PATCH "0"
   VERSION_REVISION "0"
)

project(MBot-Janny LANGUAGES CXX)

add_executable(
	MBot-Janny
	"main.cpp"
	"${PRODUCT_INFO}"
)

find_package(DiscordCoreAPI CONFIG REQUIRED)

target_link_libraries(
	MBot-Janny PUBLIC
	DiscordCoreAPI::DiscordCoreAPI
	"${VCPKG_INSTALLED_DIR}/lib/mongoc-1.0.lib"
	"${VCPKG_INSTALLED_DIR}/lib/mongocxx.lib"
	"${VCPKG_INSTALLED_DIR}/lib/bsoncxx.lib"
)

set_target_properties(
	MBot-Janny PROPERTIES 
	OUTPUT_NAME "MBot-Janny-Cpp"
	VS_GLOBAL_VcpkgEnabled true
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
	PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Output Files/$<CONFIG>"
)

if (WIN32 AND NOT VCPKG_ROOT)
	add_custom_command(TARGET MBot-Janny POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:MBot-Janny> $<TARGET_FILE_DIR:MBot-Janny>
	  COMMAND_EXPAND_LISTS
	)
endif()
