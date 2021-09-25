#doesn't include the C# editor, or Edtor Tools, at this time
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

project(HavanaEngine LANGUAGES CXX VERSION 1.0.0)

add_subdirectory(Engine)
add_subdirectory(EngineTest)