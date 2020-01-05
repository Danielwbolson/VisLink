project (VisLink)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set (SOURCEFILES
  net/Client.cpp
  net/Server.cpp
)

set (HEADERFILES
  net/Client.h
  net/Server.h
)

# --------------------- Dependencies -----------------------

find_package(Vulkan REQUIRED)
message("vk Found" ${Vulkan_FOUND} ${Vulkan_INCLUDE_DIRS} ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})
set(DEP_LIBS ${DEP_LIBS} ${Vulkan_LIBRARIES})

find_package(glfw3 REQUIRED)
set(DEP_LIBS ${DEP_LIBS} glfw)

include(${external_dir}/OpenGL/OpenGL.cmake)
useOpenGL()

# --------------------- Executable -----------------------

addSharedLibrary()

