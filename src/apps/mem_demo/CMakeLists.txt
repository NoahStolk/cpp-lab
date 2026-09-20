add_executable(mem_demo main.cpp)

target_link_libraries(mem_demo PRIVATE lab_math)
target_compile_options(mem_demo PRIVATE -Wall -Wextra)
