set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if(WIN32)
	set(
		CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} \
    /O2 \
		/Wall \
		/pipe \
		/pedantic \
		/fsized-deallocation \
    /ffunction-sections \
    /fdata-sections \
		/fdiagnostics-color=always")
else()
	set(
		CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} \
    -O2 \
		-Wall \
		-Wextra \
		-pipe \
		-pedantic \
		-fsized-deallocation \
    -ffunction-sections \
    -fdata-sections \
		-fdiagnostics-color=always \
		-Wunreachable-code \
		-Wno-attributes")
	if(NOT CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
	endif()
endif()