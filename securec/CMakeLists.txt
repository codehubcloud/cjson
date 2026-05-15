# Collect all source files for securec library
set(SECUREC_SOURCES
    src/memcpy_s.c
    src/memmove_s.c
    src/memset_s.c
    src/securecutil.c
    src/secureprintoutput_a.c
    src/snprintf_s.c
    src/sprintf_s.c
    src/strcat_s.c
    src/strcpy_s.c
    src/strncat_s.c
    src/strncpy_s.c
    src/vsnprintf_s.c
    src/vsprintf_s.c
    src/fscanf_s.c
    src/fwscanf_s.c
    src/gets_s.c
    src/scanf_s.c
    src/secureinput_a.c
    src/secureinput_w.c
    src/secureprintoutput_w.c
    src/sscanf_s.c
    src/strtok_s.c
    src/swprintf_s.c
    src/swscanf_s.c
    src/vfscanf_s.c
    src/vfwscanf_s.c
    src/vscanf_s.c
    src/vsscanf_s.c
    src/vswprintf_s.c
    src/vswscanf_s.c
    src/vwscanf_s.c
    src/wcscat_s.c
    src/wcscpy_s.c
    src/wcsncat_s.c
    src/wcsncpy_s.c
    src/wcstok_s.c
    src/wmemcpy_s.c
    src/wmemmove_s.c
    src/wscanf_s.c
)

# Create securec_lib as a static library
add_library(securec_lib STATIC ${SECUREC_SOURCES})

# Set include directories
target_include_directories(securec_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)