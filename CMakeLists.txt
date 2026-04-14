project(jngl-clay)

add_subdirectory(jngl)

add_library(ClayMan OBJECT ClayMan/clayman.cpp)
target_include_directories(ClayMan PUBLIC ClayMan ClayMan/include include)
target_compile_features(ClayMan PUBLIC cxx_std_20)

add_executable(full-example src/full-example.cpp)
target_link_libraries(full-example PRIVATE jngl ClayMan)

add_executable(jngl-clay src/main.cpp)
target_link_libraries(jngl-clay PRIVATE jngl ClayMan)

add_executable(transitions src/transitions.cpp)
target_link_libraries(transitions PRIVATE jngl)
target_include_directories(transitions PRIVATE clay include)
