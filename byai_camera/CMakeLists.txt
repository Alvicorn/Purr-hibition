include_directories(byai_camera/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(byai_camera STATIC ${MY_SOURCES})
target_link_libraries(byai_camera PRIVATE hal)

target_include_directories(byai_camera PUBLIC include)
