# Define a versão mínima necessária do CMake
cmake_minimum_required(VERSION 3.10)

# Define o nome do projeto e a linguagem (CXX para C++)
project(MeuProjeto LANGUAGES CXX)

# Define a norma do C++ (ex: C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adiciona o executável e associa-o aos ficheiros fonte
add_executable(meu_executavel main.cpp)
