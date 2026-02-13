add_cmocka_test(test_math
                SOURCES test_math.c
                LINK_LIBRARIES lib_darktable cmocka)

add_cmocka_test(test_ai_core
                SOURCES test_ai_core.c
                LINK_LIBRARIES lib_darktable cmocka)

# Windows: main-method requires the wrapper provided by lib-darktable
if(WIN32)
    target_link_libraries(test_math PRIVATE lib_darktable)
    _copy_required_library(test_math lib_darktable)
endif(WIN32)
