CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(SPPM CXX)

 
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Ofast -w -mavx2")

find_package(OpenMP REQUIRED)


IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

ADD_SUBDIRECTORY(deps/vecmath)

SET(SPPM_SOURCES
        src/image.cpp
        src/main.cpp
        #src/lodepng.cpp
        src/mesh.cpp
        src/scene_parser.cpp
        src/curve.cpp
        src/material.cpp
        src/render.cpp
        src/utils.cpp
        src/sppm.cpp
		)

SET(SPPM_INCLUDES
        include/bvh.hpp
        include/camera.hpp
        include/classical_object.hpp
        include/group.hpp
        include/hit.hpp
        include/image.hpp
        #include/lodepng.h
        include/stb_image_write.h
        include/stb_image.h
        #include/svpng.hpp
        include/light.hpp
        include/material.hpp
        include/mesh.hpp
        include/object3d.hpp
        #include/plane.hpp
        include/ray.hpp
        include/render.hpp
        include/scene_parser.hpp
        include/sppm.hpp
        #include/sphere.hpp
        include/tiny_obj_loader.h
        include/transform.hpp
        include/triangle.hpp
        include/texture.hpp
        include/utils.hpp
        )

SET(CMAKE_CXX_STANDARD 14)

ADD_EXECUTABLE(${PROJECT_NAME} ${SPPM_SOURCES} ${SPPM_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath OpenMP::OpenMP_CXX)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)
