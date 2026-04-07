# Unity Test Framework
set(UNITY_SOURCES unity/unity.c)
set(UNITY_HEADERS unity/unity.h unity/unity_internals.h)

# Test sources
set(TEST_SOURCES
    test_fixture.c
    main.c
    test_util.c
    test_pos.c
    test_language.c
    test_text.c
    test_definition.c
    test_info.c
    test_reader.c
)

# Create test executable
add_executable(bgl_tests
    ${UNITY_SOURCES}
    ${TEST_SOURCES}
)

target_include_directories(bgl_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/unity
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(bgl_tests PRIVATE bgl ZLIB::ZLIB)

# Enable testing
add_test(NAME UnityTestSuite COMMAND bgl_tests)
