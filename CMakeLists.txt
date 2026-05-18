cmake_minimum_required(VERSION 3.10)
project(ray_tracer)

add_executable(ray_tracer
        main.cpp
        camera.cpp
        hierarchy.cpp
        flat_shader.cpp
        parse.cpp
        phong_shader.cpp
        plane.cpp
        reflective_shader.cpp
        render_world.cpp
        sphere.cpp
        box.cpp
        mesh.cpp
        noise.cpp
)

add_executable(noise_preview
        noise_preview.cpp
        noise.cpp
)

target_link_libraries(ray_tracer PRIVATE)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()
