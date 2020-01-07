project (ExampleApp)

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


set(DEP_LINK_DIRS ${DEP_LINK_DIRS} ${external_dir}/SandBox/build/install/lib)
set(DEP_INCLUDES ${DEP_INCLUDES} 
	${external_dir}/SandBox/src/src
	${external_dir}/SandBox/cmake/framework/external/glm/src
)

include(${external_dir}/OpenGL/OpenGL.cmake)
useOpenGL()

# --------------------- Executable -----------------------

addStandardExecutable()

