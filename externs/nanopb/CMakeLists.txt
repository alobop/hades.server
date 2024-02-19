include(FetchContent)

FetchContent_Declare(extern.nanopb
                     GIT_REPOSITORY https://github.com/nanopb/nanopb.git
                     GIT_TAG        1f0c2e19c661f18dd88428858b8e965a26589e03)

FetchContent_Populate(extern.nanopb)
FetchContent_GetProperties(extern.nanopb SOURCE_DIR EXTERNS_NANOPB_SOURCE_DIR)
message(STATUS ${EXTERNS_THREADX_SOURCE_DIR})

add_library(extern.lib.nanopb STATIC 
    ${EXTERNS_NANOPB_SOURCE_DIR}/pb_common.c
    ${EXTERNS_NANOPB_SOURCE_DIR}/pb_decode.c
    ${EXTERNS_NANOPB_SOURCE_DIR}/pb_encode.c
)

target_include_directories(extern.lib.nanopb
    PUBLIC
        ${EXTERNS_NANOPB_SOURCE_DIR}/
)
