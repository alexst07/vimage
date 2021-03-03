cmake_minimum_required (VERSION 2.8)
project(erised C CXX)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(PNG REQUIRED)

include_directories(${BASE_IMG_LIB}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/)

file(GLOB LIB_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_library(volimg SHARED ${LIB_SRC_FILES})

message("png lib: ${PNG_LIBRARIES}")
target_link_libraries (volimg LINK_PUBLIC ${PNG_LIBRARIES})

add_subdirectory(tests/)
