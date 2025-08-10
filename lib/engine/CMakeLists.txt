cmake_minimum_required(VERSION 3.25)
project(engine VERSION 0.1.0 LANGUAGES CXX)

add_library(engine STATIC
    src/game.cpp
    src/run.cpp
)

target_compile_features(engine PUBLIC cxx_std_20)
target_include_directories(engine PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
      $<INSTALL_INTERFACE:include>)

# Raylib dependency is private to the engine. Unless we want to expose types directly
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET CONFIG)
if(NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
  )
  FetchContent_MakeAvailable(raylib)
endif()
target_link_libraries(engine PRIVATE raylib) # make PUBLIC only if engine's public headers include raylib


# Optional install/export for future find_package(engine)
if(PROJECT_IS_TOP_LEVEL)
  include(CMakePackageConfigHelpers)
  install(TARGETS engine EXPORT engineTargets
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include)
  install(DIRECTORY include/ DESTINATION include)
  install(EXPORT engineTargets NAMESPACE engine:: DESTINATION lib/cmake/engine)
endif()
