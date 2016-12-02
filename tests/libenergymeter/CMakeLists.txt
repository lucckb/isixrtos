project (Test)
enable_testing()
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE(Threads)
find_package(GTest REQUIRED)
find_package(FFTW REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${FFTW3_INCLUDE_DIRS})

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


file( GLOB emlib_SRC "../../libenergymeter/src/*.cpp" "../../libenergymeter/src/*.c" )
file( GLOB unittest_SRC "*.cpp" "*.c" )
file( GLOB foundation_SRC "../../libfoundation/util*.c*"
        "../../libfoundation/tiny_printf.c"
    )


add_executable ( emtest  ${unittest_SRC}
	${foundation_SRC}
	${emlib_SRC} 
    )

target_link_libraries(emtest ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(emtest ${GTEST_BOTH_LIBRARIES})
target_link_libraries(emtest ${FFTW_LIBRARIES})

include_directories ("${PROJECT_SOURCE_DIR}/include" 
    "../../libfoundation/include" 
	"../../libenergymeter/include")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -Wextra -Wall -Wstack-protector -mtune=native" )
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11 -Wextra -Wstack-protector -mtune=native" )
SET(CMAKE_CXX_FLAGS_DEBUG "-DCONFIG_LIBFOUNDATION_ENV_FS_DEBUG -O0 -g")
