enable_testing()

function(tlct_add_test name srcs)
    add_executable(${name} EXCLUDE_FROM_ALL ${srcs})
    target_link_libraries(${name} PRIVATE tlct doctest)
    target_compile_definitions(${name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

    copy_dlls_if_needed(${name})
    doctest_discover_tests(${name})
endfunction()

tlct_add_test(test_static_math "test_static_math.cpp")
tlct_add_test(test_common_cfg "test_common_cfg.cpp")
tlct_add_test(test_tspc_cfg "test_tspc_cfg.cpp")
