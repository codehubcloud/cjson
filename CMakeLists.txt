cmake_minimum_required(VERSION 3.15)

project(cjson_response C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(cjson_response STATIC
    cjson_response.c
)

target_include_directories(cjson_response
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(cjson_response
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_executable(main_response_only_example
    main_response_only_example.c
)

target_link_libraries(main_response_only_example
    PRIVATE
        cjson_response
)

target_compile_options(main_response_only_example
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
