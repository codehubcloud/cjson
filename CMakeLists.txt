cmake_minimum_required(VERSION 3.15)

project(cjson_response_example C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Add securec subdirectory
add_subdirectory(securec)

# Create securec_stub library (stub implementation if needed)
add_library(securec_stub INTERFACE)

target_include_directories(securec_stub
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create cjson_response library
add_library(cjson_response STATIC
    cjson_response.c
)

target_include_directories(cjson_response
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cjson_response
    PUBLIC
        securec_lib
)

# Create main executable
add_executable(main_response_only_example
    main_response_only_example.c
    ft_result_test.c
    ft_result_test_voltage.c
    ft_result_test_basictype.c
)

target_link_libraries(main_response_only_example
    PRIVATE
        cjson_response
)
