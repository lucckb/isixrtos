project (Test)
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE(Threads)

file( GLOB gsmlib_SRC "../../libgsm/src/*.cpp" "../../libgsm/src/*.c" )
file( GLOB unittest_SRC "*.cpp" "*.c" )
file( GLOB foundation_SRC "../../libfoundation/fs_env.cpp"
        "../../libfoundation/i2c_eeprom.cpp"
        "../../libfoundation/util*.c*"
        "../../libfoundation/tiny_printf.c"
    )


add_executable ( ftest  ${unittest_SRC}
	${foundation_SRC}
	${gsmlib_SRC} 
    )
include_directories ("${PROJECT_SOURCE_DIR}/include" 
    "../../libfoundation/include" 
	"../../libgsm/include")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wextra" )
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11 -Wextra" )
SET(CMAKE_CXX_FLAGS_DEBUG "-DCONFIG_LIBFOUNDATION_ENV_FS_DEBUG -O0 -g")
TARGET_LINK_LIBRARIES(ftest ${CMAKE_THREAD_LIBS_INIT})
