cmake_minimum_required(VERSION 3.10)
project(AdmsTcpServer VERSION 1.0 LANGUAGES CXX)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Required Dependencies
find_package(Threads REQUIRED)

if(WIN32)
    # Check if user provided a precompiled libcurl SDK path (e.g. -DCURL_ROOT=C:/path/to/curl)
    if(CURL_ROOT OR CURL_DIR OR DEFINED ENV{CURL_ROOT} OR DEFINED ENV{CURL_DIR})
        # Use the precompiled libcurl package provided
        find_package(CURL REQUIRED)
    else()
        # On Windows, build libcurl from source using the built-in Schannel SSL backend
        # This avoids the need to install OpenSSL or download manual DLLs on Windows.
        set(BUILD_CURL_EXE OFF CACHE INTERNAL "disable curl exe build")
        set(BUILD_SHARED_LIBS ON CACHE INTERNAL "build curl as shared library")
        set(CURL_USE_SCHANNEL ON CACHE INTERNAL "use Windows native Schannel for SSL")
        set(CURL_USE_OPENSSL OFF CACHE INTERNAL "disable openssl")
        set(CURL_ENABLE_SSL ON CACHE INTERNAL "enable SSL")
        
        include(FetchContent)
        FetchContent_Declare(
            curl
            URL https://github.com/curl/curl/archive/refs/tags/curl-8_8_0.tar.gz
        )
        FetchContent_MakeAvailable(curl)
        
        if(NOT TARGET CURL::libcurl)
            add_library(CURL::libcurl ALIAS libcurl)
        endif()
    endif()
else()
    # On Linux, use the fast, pre-installed system libcurl package
    find_package(CURL REQUIRED)
endif()

# Fetch Standalone Asio Headers
include(FetchContent)
FetchContent_Declare(
    asio_headers
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
)
FetchContent_MakeAvailable(asio_headers)

# Define the source files for the executable
add_executable(adms_server
    src/main.cpp
    src/proto/parser.cpp
    src/http/poster.cpp
    src/log/logger.cpp
    src/config.cpp
)

# Include directories and compile definitions
target_include_directories(adms_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${asio_headers_SOURCE_DIR}/asio/include
)

target_compile_definitions(adms_server PRIVATE ASIO_STANDALONE)

# Enable strict warnings and link time optimization (O3 for release speed)
target_compile_options(adms_server PRIVATE -Wall -Wextra -O3)

# Link the required libraries (libcurl and threading)
target_link_libraries(adms_server PRIVATE 
    CURL::libcurl 
    Threads::Threads
)

# Configuration for Packaging / Local Distribution Folder
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Default packaging directory" FORCE)
endif()

# Install target files
install(TARGETS adms_server RUNTIME DESTINATION .)

if(WIN32)
    # If libcurl was compiled from source (on Windows), package the DLL alongside the exe
    if(TARGET libcurl)
        install(TARGETS libcurl RUNTIME DESTINATION .)
    endif()
endif()

# Package the configuration template
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/.env.example DESTINATION .)
