cmake_minimum_required(VERSION 3.10)
project(VectorStorm CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Threads REQUIRED)

add_executable(game
    main.cpp
    src/Audio/SoundEngine.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/ParticleSystem.cpp
    src/Graphics/Starfield.cpp
    src/Graphics/CGLab.cpp
    src/Entities/Powerup.cpp
    src/Core/SaveSystem.cpp
)

target_include_directories(game PRIVATE include ${GLUT_INCLUDE_DIR})
target_link_libraries(game PRIVATE ${GLUT_LIBRARIES} OpenGL::GLU OpenGL::GL Threads::Threads)

if(WIN32)
    target_link_libraries(game PRIVATE winmm)
endif()

if(MINGW)
    # Static C++ runtime + winpthread: game.exe then only needs libfreeglut.dll
    target_link_options(game PRIVATE -static-libgcc -static-libstdc++
                        -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()
