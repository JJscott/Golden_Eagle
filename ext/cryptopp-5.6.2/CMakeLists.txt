project(CRYPTOPP)
# TODO update minimum version for .asm support for Visual Studio generators
cmake_minimum_required (VERSION 2.8.3)

# GCC: require to build with -pthread, and appropriate -march for enabled intrinsics

# Visual Studio (2013 anyway):
# CMake (2.8.11) does not properly enable building of MASM sources.
# - From Solution Explorer, project 'cryptopp' > right-click > Build Dependencies >
#   Build Customizations > [X] masm
# - From Solution Explorer, files 'x64dll.asm' and 'x64masm.asm' > right-click > Properties >
#   (select 'All Confgurations' from the top-left drop-down) >
#   Configuration Properties > General > Item Type = 'Microsoft Macro Assembler'
# If CMake needs to recreate the cryptopp project, these settings get destroyed.

# user options
option(CRYPTOPP_ASM "CRYPTOPP_ASM" ON)
option(CRYPTOPP_SSE2 "CRYPTOPP_SSE2" ON)
option(CRYPTOPP_SSSE3 "CRYPTOPP_SSSE3" OFF)
option(CRYPTOPP_AESNI "CRYPTOPP_AESNI" OFF)

file(GLOB cryptopp_src "./src/CryptoPP/*.cpp")
file(GLOB cryptopp_hdr "./src/CryptoPP/*.h")

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") AND CMAKE_CL_64 AND CRYPTOPP_ASM)
	enable_language(ASM_MASM)
	file(GLOB cryptopp_asm "./src/CryptoPP/*.asm")
	set(cryptopp_src ${cryptopp_src} ${cryptopp_asm})
	message(STATUS "\n\n****************************\n"
		"CryptoPP Visual Studio project requires manual intervention.\n"
		"See directions at top of CryptoPP's primary CMakeLists.txt.\n"
		"THIS APPLIES EVEN IF YOU'VE DONE IT BEFORE.\n"
		"****************************\n\n"
	)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-integrated-as")	
endif()

add_library(cryptopp ${cryptopp_src} ${cryptopp_hdr})

set_target_properties(cryptopp
    PROPERTIES
    LINKER_LANGUAGE CXX
)

# list libs that must be linked with cryptopp
list(APPEND CRYPTOPP_LIBRARIES_impl)
if(WIN32)
	list(APPEND CRYPTOPP_LIBRARIES_impl ws2_32)
endif()

# list macros that must be defined with cryptopp
list(APPEND CRYPTOPP_DEFINITIONS_impl)
if(NOT CRYPTOPP_ASM)
	list(APPEND CRYPTOPP_DEFINITIONS_impl -DCRYPTOPP_DISABLE_ASM)
endif()
if(NOT CRYPTOPP_SSE2)
	list(APPEND CRYPTOPP_DEFINITIONS_impl -DCRYPTOPP_DISABLE_SSE2)
endif()
if(NOT CRYPTOPP_SSSE3)
	list(APPEND CRYPTOPP_DEFINITIONS_impl -DCRYPTOPP_DISABLE_SSSE3)
endif()
if(NOT CRYPTOPP_AESNI)
	list(APPEND CRYPTOPP_DEFINITIONS_impl -DCRYPTOPP_DISABLE_AESNI)
endif()

# export
set(CRYPTOPP_LIBRARIES ${CRYPTOPP_LIBRARIES_impl} CACHE STRING "CryptoPP required libraries" FORCE)
set(CRYPTOPP_DEFINITIONS ${CRYPTOPP_DEFINITIONS_impl} CACHE STRING "CryptoPP required definitions" FORCE)

add_definitions(${CRYPTOPP_DEFINITIONS})








