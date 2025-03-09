include_directories(hal/include)
file(GLOB MY_SOURCES "src/*.c")

# find_library(GPIOD_LIBRARY gpiod) // added but commented out because doesn't seem like it's necessary but unsure
add_library(hal STATIC ${MY_SOURCES})

target_include_directories(hal PUBLIC include)
target_link_libraries(hal LINK_PRIVATE gpiod) # May need to change to HAL lib.