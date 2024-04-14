include_guard()

include(GodotJoltConfigurations)
include(GodotJoltOptions)

# Set EP_BASE so we get a nicer directory structure for external projects
set_directory_properties(PROPERTIES EP_BASE ${PROJECT_BINARY_DIR}/External)

if(PROJECT_IS_TOP_LEVEL)
	# Group targets into folders for any IDE that supports it (like Visual Studio)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	# Ensure we output to the `bin` directory when building from Visual Studio
	set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD TRUE)
endif()

if(NOT DEFINED GDJ_TARGET_ARCHITECTURES)
	if(APPLE)
		set(GDJ_TARGET_ARCHITECTURES ${CMAKE_OSX_ARCHITECTURES})
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES [[^(i386|i686|x86)$]])
		set(GDJ_TARGET_ARCHITECTURES x86)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES [[^(AMD64|amd64|x86_64)$]])
		set(GDJ_TARGET_ARCHITECTURES x64)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES [[^aarch64$]])
		set(GDJ_TARGET_ARCHITECTURES arm64)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES [[^armv7-a$]])
		set(GDJ_TARGET_ARCHITECTURES arm32)
	else()
		message(FATAL_ERROR "Unhandled system processor: '${CMAKE_SYSTEM_PROCESSOR}'.")
	endif()
endif()

if(MSVC AND DEFINED ENV{VSCMD_ARG_TGT_ARCH})
	if(NOT $ENV{VSCMD_ARG_TGT_ARCH} STREQUAL ${GDJ_TARGET_ARCHITECTURES})
		message(FATAL_ERROR
			"Mismatched target architectures. The current Visual Studio environment is set up for "
			"the '$ENV{VSCMD_ARG_TGT_ARCH}' architecture, but this configuration targets the "
			"'${GDJ_TARGET_ARCHITECTURES}' architecture.\nPlease make sure you launch the "
			"Visual Studio command prompt appropriate for the intended target architecture.\n"
			"You should delete the build directory generated by this command before trying again."
		)
	endif()
endif()

macro(is_targeting instruction_sets output_variable)
	if(GDJ_X86_INSTRUCTION_SET MATCHES "^(${instruction_sets})$")
		set(${output_variable} TRUE)
	else()
		set(${output_variable} FALSE)
	endif()
endmacro()

is_targeting(AVX512 GDJ_USE_AVX512)
is_targeting(AVX512|AVX2 GDJ_USE_AVX2)
is_targeting(AVX512|AVX2 GDJ_USE_BMI1)
is_targeting(AVX512|AVX2 GDJ_USE_FMA3)
is_targeting(AVX512|AVX2 GDJ_USE_F16C)
is_targeting(AVX512|AVX2|AVX GDJ_USE_AVX)
is_targeting(AVX512|AVX2|AVX GDJ_USE_SSE4_2)
is_targeting(AVX512|AVX2|AVX|SSE2 GDJ_USE_SSE2)