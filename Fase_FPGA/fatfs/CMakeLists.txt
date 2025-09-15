add_library(fatfs
    ff.c
    diskio.c
    ffunicode.c
)

target_include_directories(fatfs PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(fatfs
    pico_stdlib
    hardware_spi
)

