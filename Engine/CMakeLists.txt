IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
file(GLOB PROJECT_SOURCE_FILES CONFIGURE_DEPENDS Components/*.cpp Content/*.cpp Core/*.cpp EngineAPI/*.cpp Graphics/*.cpp Graphics/Direct3D12/*.cpp Graphics/Vulkan/*.cpp Graphics/OpenGL/*.cpp Platforms/*.cpp Utilities/*.cpp)
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
file(GLOB PROJECT_SOURCE_FILES CONFIGURE_DEPENDS Components/*.cpp Content/*.cpp Core/*.cpp EngineAPI/*.cpp Graphics/*.cpp Graphics/Vulkan/*.cpp Graphics/OpenGL/*.cpp Platforms/*.cpp Utilities/*.cpp)
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
#Apple stuff here
ENDIF()

add_library(engine ${PROJECT_SOURCE_FILES})

target_link_libraries(engine GL X11 dl vulkan)

target_include_directories(engine PUBLIC Common Components)