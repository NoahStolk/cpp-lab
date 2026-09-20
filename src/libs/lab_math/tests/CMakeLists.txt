add_executable(lab_math_tests vec.tests.cpp)

target_link_libraries(lab_math_tests PRIVATE lab_math)
target_compile_options(lab_math_tests PRIVATE -Wall -Wextra)

add_test(NAME lab_math_tests COMMAND lab_math_tests)
