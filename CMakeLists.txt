cmake_minimum_required(VERSION 3.27)
project(mtfind)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(mtfind main.cpp
        Grep.cpp
        Grep.h)

#-Dsanitize_type:STRING=Memory
if (sanitize_type)
    if (${sanitize_type} STREQUAL Memory)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak")
    elseif (${sanitize_type} STREQUAL Thread)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
    endif ()
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-error=unused-parameter -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Werror -Wextra")

target_link_libraries(mtfind PRIVATE Boost::program_options)