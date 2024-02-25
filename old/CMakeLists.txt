cmake_minimum_required(VERSION 3.24)
project(ChessBot)

set(CMAKE_CXX_STANDARD 23)

add_executable(ChessBot ../main.cpp "../source code/BoardState.cpp" "../source code/Move.cpp" "../source code/Game.cpp" "../source code/Square.cpp")
