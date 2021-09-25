file(GLOB PROJECT_SOURCE_FILES CONFIGURE_DEPENDS Content/*.cpp Core/*.cpp EngineAPI/*.cpp Graphics/*.cpp Platforms/*.cpp Utilities/*.cpp)

add_library(engine ${PROJECT_SOURCE_FILES})

target_link_libraries(engine GL X11)

target_include_directories(engine PUBLIC Common)