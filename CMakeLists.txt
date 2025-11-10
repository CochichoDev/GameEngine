cmake_minimum_required(VERSION 3.16)
project(GameEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# SDL3
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
# SDL3_net
add_subdirectory(vendored/SDL_net EXCLUDE_FROM_ALL)

set(SOURCES
    src/main.cpp
)

set(HEADERS
    include/
)

function(add_variant name define)
    add_executable(${name} ${SOURCES} ${HEADERS})

    target_include_directories(${name} PRIVATE include)
    target_link_libraries(${name} PRIVATE 
        SDL3::SDL3
        SDL3_net::SDL3_net
    )

    if(define)
        target_compile_definitions(${name} PRIVATE ${define})
    endif()

    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra -pedantic -g)
    endif()
endfunction()

add_variant(${PROJECT_NAME}_client "")
add_variant(${PROJECT_NAME}_server SERVER)
