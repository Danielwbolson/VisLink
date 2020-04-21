project (SimpleClient)

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)

# --------------------- Source -----------------------

set (SOURCEFILES
  src/main.cpp
)

set (HEADERFILES
)

# --------------------- Dependencies -----------------------


set(PROJECT_DEPS ${PROJECT_DEPS} VisLink)
set(DEP_LIBS ${DEP_LIBS} VisLink)

add_definitions(-DGLM_FORCE_INLINE)
include_directories(${external_dir}/glm/src)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
	${external_dir}/SandBox/build/install/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
	${external_dir}/SandBox/src/src
	${external_dir}/SandBox/cmake/framework/external/glm/src
	${external_dir}/SandBox/src/cmake/framework/external/stb/src
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  ${external_dir}/SandBox/src/cmake/framework/external/glfw3/build/install/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  ${external_dir}/SandBox/src/cmake/framework/external/glfw3/build/install/include
)
if(MSVC)
  set(DEP_LIBS ${DEP_LIBS} optimized glfw3.lib debug glfw3d.lib PARENT_SCOPE)
else()
  set(DEP_LIBS ${DEP_LIBS} optimized libglfw3.a debug libglfw3d.a PARENT_SCOPE)
endif()

include(${external_dir}/OpenGL/OpenGL.cmake)
useOpenGL()
useGLEW()

# --------------------- Executable -----------------------

addStandardExecutable()

