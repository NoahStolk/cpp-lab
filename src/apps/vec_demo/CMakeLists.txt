add_executable(vec_demo main.cpp)

target_link_libraries(vec_demo PRIVATE lab_math)
target_compile_options(vec_demo PRIVATE -Wall -Wextra)
