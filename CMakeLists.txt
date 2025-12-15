-cmake_minimum_required(VERSION 3.5)
project(exercicio C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Place build output binaries under build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SRC
    exercicio.c
)

add_executable(exercicio ${SRC})

target_include_directories(exercicio PRIVATE ${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

target_compile_options(exercicio PRIVATE -Wall -Wextra -O2)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_compile_options(exercicio PRIVATE -Wno-unused-result)
endif()

install(TARGETS exercicio RUNTIME DESTINATION bin)
