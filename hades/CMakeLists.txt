set(TARGET lib.hades)

add_library(${TARGET}
    STATIC
    ./src/hades.c

    ./src/commands.c
    ./src/logger.c
    ./src/request.c
)

target_include_directories(
    ${TARGET} 
    PUBLIC
        ./inc
)

target_link_libraries(
    ${TARGET}
    PUBLIC
        extern.lib.nanopb
)
