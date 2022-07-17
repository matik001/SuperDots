CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(Kropki C)

SET(CMAKE_C_STANDARD 11)

# Use the package PkgConfig to detect GTK+ headers/library files
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

# Add an executable compiled from main.c

file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} "./src/*.c")
add_executable(Kropki ${SOURCE_FILES})

# Link the target to the GTK+ libraries
TARGET_LINK_LIBRARIES(Kropki ${GTK3_LIBRARIES})

#TARGET_COMPILE_OPTIONS(Tic_Tac_Toe_C PRIVATE -Werror -Wall -Wextra)