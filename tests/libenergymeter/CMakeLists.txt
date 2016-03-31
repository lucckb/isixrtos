project (Test)
enable_testing()
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE(Threads)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file( GLOB emlib_SRC "../../libenergymeter/src/*.cpp" "../../libenergymeter/src/*.c" )
file( GLOB unittest_SRC "*.cpp" "*.c" )
file( GLOB foundation_SRC "../../libfoundation/util*.c*"
        "../../libfoundation/tiny_printf.c"
    )


add_executable ( emtest  ${unittest_SRC}
	${foundation_SRC}
	${emlib_SRC} 
    )

target_link_libraries(emtest ${GTEST_BOTH_LIBRARIES})

include_directories ("${PROJECT_SOURCE_DIR}/include" 
    "../../libfoundation/include" 
	"../../libenergymeter/include")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wextra" )
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11 -Wextra" )
SET(CMAKE_CXX_FLAGS_DEBUG "-DCONFIG_LIBFOUNDATION_ENV_FS_DEBUG -O0 -g")
TARGET_LINK_LIBRARIES(emtest ${CMAKE_THREAD_LIBS_INIT})
