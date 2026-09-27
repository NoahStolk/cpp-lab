add_executable(game_demo
        main.cpp
        ui.cpp
        cell.cpp)

target_compile_options(game_demo PRIVATE -Wall -Wextra)
