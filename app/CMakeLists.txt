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

# --------------------- Executable -----------------------

addStandardExecutable()

