cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(game-pacman LANGUAGES CXX)

# use the engine from monorepo
add_subdirectory(lib/engine)

# Output Layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Executable
add_executable(pacman src/main.cpp)
target_compile_features(pacman PRIVATE cxx_std_20)
target_link_libraries(pacman PRIVATE engine)

set_target_properties(pacman PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Optional warnings/sanitizers
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  foreach(t engine pacman)
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_compile_options(${t} PRIVATE -fsanitize=address,undefined)
      target_link_options(${t} PRIVATE -fsanitize=address,undefined)
    endif()
  endforeach()
endif()
