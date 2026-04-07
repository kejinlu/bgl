cmake_minimum_required(VERSION 3.14)
project(libbgl VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------- Options ----------
option(BGL_BUILD_TESTS "Build tests" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# ---------- Dependencies ----------
find_package(ZLIB REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Iconv REQUIRED)

# ---------- Library ----------
file(GLOB BGL_SOURCES src/*.c)
file(GLOB BGL_HEADERS src/*.h)

set(BGL_PUBLIC_HEADERS
    src/bgl_reader.h
    src/bgl_definition.h
    src/bgl_info.h
    src/bgl_language.h
    src/bgl_pos.h
    src/bgl_text.h
)

add_library(bgl ${BGL_SOURCES})
target_include_directories(bgl
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include>
)
target_link_libraries(bgl PUBLIC ZLIB::ZLIB LibXml2::LibXml2 Iconv::Iconv)

# Platform-specific link libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(bgl PUBLIC m)
endif()

# MSVC: suppress POSIX function name warnings (strdup, etc.)
if(MSVC)
    target_compile_definitions(bgl PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(bgl PRIVATE /W4)
else()
    target_compile_options(bgl PRIVATE -Wall -Wextra -pedantic)
endif()

# ---------- Tests ----------
if(BGL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------- Install ----------
include(GNUInstallDirs)

install(TARGETS bgl
    EXPORT bglTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES ${BGL_PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bgl)

install(EXPORT bglTargets
    FILE bglTargets.cmake
    NAMESPACE bgl::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bgl
)
