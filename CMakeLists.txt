# Define a versão mínima necessária do CMake
cmake_minimum_required(VERSION 3.10)

# Define o nome do projeto e a linguagem (CXX para C++)
project(KineFlow LANGUAGES CXX)

# Define a norma do C++ (ex: C++17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Procura todos os ficheiros .cpp na pasta kineflow
file(GLOB KINEFLOW_SOURCES "kineflow/*.hpp")
file(GLOB KINEFLOW_DEFINITIONS "kineflow/*.cpp")

# Adiciona o executável com o main e os fontes da pasta
add_executable(engineFlow main.cpp ${KINEFLOW_SOURCES} ${KINEFLOW_DEFINITIONS})

# Torna a pasta RAIZ do projeto visível para os includes
# Assim, #include <kineflow/scene.hpp> funcionará
target_include_directories(engineFlow PRIVATE ${CMAKE_SOURCE_DIR})
