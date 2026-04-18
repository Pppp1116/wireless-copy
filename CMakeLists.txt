cmake_minimum_required(VERSION 3.20)

project(rdbg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(asio CONFIG QUIET)
find_package(Threads REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(PROJECT_TARGET "${PROJECT_NAME}")

option(ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during the build" OFF)
option(ENABLE_AGGRESSIVE_RELEASE "Enable aggressive release optimizations" OFF)

set(SANITIZER "none" CACHE STRING
    "Sanitizer set: none, address, undefined, address,undefined, thread")
set_property(CACHE SANITIZER PROPERTY STRINGS
    none address undefined "address,undefined" thread)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(ENABLE_CLANG_TIDY)
    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "ENABLE_CLANG_TIDY=ON but clang-tidy was not found")
    endif()

    set(CLANG_TIDY_COMMAND
        "${CLANG_TIDY_EXE}"
        "-p=${CMAKE_BINARY_DIR}"
        "--use-color"
    )

    if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
        list(APPEND CLANG_TIDY_COMMAND
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        )
    endif()

    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.cc
    src/*.cxx
)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
    src/*.h
    src/*.hpp
    src/*.hh
    src/*.hxx
)

if(NOT PROJECT_SOURCES)
    message(FATAL_ERROR "No C++ source files were found under ${CMAKE_SOURCE_DIR}/src")
endif()

add_executable(${PROJECT_TARGET}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_compile_features(${PROJECT_TARGET} PRIVATE cxx_std_26)

target_include_directories(${PROJECT_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/include")
    target_include_directories(${PROJECT_TARGET} PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
    )
endif()

if(asio_FOUND)
    target_link_libraries(${PROJECT_TARGET} PRIVATE asio::asio Threads::Threads)
else()
    message(STATUS "asio package not found via CMake config; assuming standalone header-only Asio is available in the compiler include path")
    target_link_libraries(${PROJECT_TARGET} PRIVATE Threads::Threads)
endif()

set(PROJECT_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wformat=2
    -Wformat-security
    -Wmissing-declarations
    -Wmissing-field-initializers
    -Wimplicit-fallthrough
    -Wnull-dereference
    -Wundef
    -Wcast-align
    -Wswitch-enum
    -Wuninitialized
    -Werror=format-security
    -Wunused
    -Wunreachable-code
    -Wstrict-aliasing
    -Wredundant-move      
    -Wpessimizing-move 
    -Wcast-qual
    -Winline   
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND PROJECT_WARNING_FLAGS
        -fcolor-diagnostics
        -fdiagnostics-format=clang
        -fdiagnostics-show-option
        -fdiagnostics-show-template-tree
        -fdiagnostics-absolute-paths
        -fmacro-backtrace-limit=1
        -ferror-limit=10
        -Wunsafe-buffer-usage
        -Wold-style-cast
        -Wextra-semi
        -Wnon-virtual-dtor
        -Woverloaded-virtual
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND PROJECT_WARNING_FLAGS
        -fdiagnostics-color=always
        -fdiagnostics-show-option
    )
endif()
target_compile_options(${PROJECT_TARGET} PRIVATE
    ${PROJECT_WARNING_FLAGS}
    -fstack-protector-strong
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

target_compile_definitions(${PROJECT_TARGET} PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:_GLIBCXX_ASSERTIONS>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:_GLIBCXX_ASSERTIONS>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG>
)

if(ENABLE_WARNINGS_AS_ERRORS)
    target_compile_options(${PROJECT_TARGET} PRIVATE
        -Werror
        -pedantic-errors
    )
endif()

set(SAN_FLAGS)

if(SANITIZER STREQUAL "address")
    list(APPEND SAN_FLAGS
        -fsanitize=address
        -fsanitize-address-use-after-scope
    )
elseif(SANITIZER STREQUAL "undefined")
    list(APPEND SAN_FLAGS
        -fsanitize=undefined
        -fno-sanitize-recover=undefined
    )
elseif(SANITIZER STREQUAL "address,undefined")
    list(APPEND SAN_FLAGS
        -fsanitize=address,undefined
        -fsanitize-address-use-after-scope
        -fno-sanitize-recover=undefined
    )
elseif(SANITIZER STREQUAL "thread")
    list(APPEND SAN_FLAGS
        -fsanitize=thread
    )
elseif(NOT SANITIZER STREQUAL "none")
    message(FATAL_ERROR "Unsupported SANITIZER='${SANITIZER}'")
endif()

if(SAN_FLAGS)
    target_compile_options(${PROJECT_TARGET} PRIVATE
        -fno-omit-frame-pointer
        ${SAN_FLAGS}
    )
    target_link_options(${PROJECT_TARGET} PRIVATE
        ${SAN_FLAGS}
    )
endif()

if(ENABLE_AGGRESSIVE_RELEASE)
    if(SAN_FLAGS)
        message(FATAL_ERROR "ENABLE_AGGRESSIVE_RELEASE cannot be used with sanitizers")
    endif()

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

    if(IPO_SUPPORTED)
        set_property(TARGET ${PROJECT_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO not supported: ${IPO_ERROR}")
    endif()

    target_compile_options(${PROJECT_TARGET} PRIVATE
        -O3
        -DNDEBUG
        -flto
    )

    target_link_options(${PROJECT_TARGET} PRIVATE
        -O3
        -flto
    )
endif()
