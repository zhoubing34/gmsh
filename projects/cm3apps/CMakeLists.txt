cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose build type")
else(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose build type")
endif(DEFINED CMAKE_BUILD_TYPE)

# variable for other projects (not include NonLinearMechSolver)
set(CM3APPS TRUE)

if(CMAKE_BUILD_TYPE MATCHES "debug")
  set(INDEBUG TRUE)
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(INDEBUG TRUE)
else()
  set(INDEBUG FALSE)
endif(CMAKE_BUILD_TYPE MATCHES "debug")

# find gdb to be able to launch debugger
find_program(GDB_EXECUTABLE gdb)

project(cm3apps)

# for nightly build disable all the warnings
option(DISABLE_WARNINGS "disable the warnings" OFF)
if(DISABLE_WARNINGS)
  message("Warning disabled")
  add_definitions("-w")
  # to disable GMSH python bindings generated by swig
  # according to SWIG documentation it is not possible to disable
  # all warnings at once so list herein all the warnings appearing 
  # while compiling the bindings 
  set(DISABLE_SWIG_WARNINGS_FLAG "-w312,314,325,362,389,401,503,509")
endif(DISABLE_WARNINGS)
# includes or not the other projects (default no project)
option(ENABLE_DGSHELL "dgshell" OFF)
option(ENABLE_DG3D "dG3D" OFF)
option(ENABLE_MSCH "msch" OFF)

#  NonLinearSolver is mandatory
add_subdirectory(../NonLinearSolver "${CMAKE_CURRENT_BINARY_DIR}/NonLinearSolver")
include_directories(. ../../Common ../../Numeric ../../Geo ../../Mesh
   ../../Solver ../../Post ../../Plugin ../../Graphics ../NonLinearSolver/internalPoints ../NonLinearSolver/nlsolver ../NonLinearSolver ../NonLinearSolver/Interface ../NonLinearSolver/contact ../NonLinearSolver/materialLaw ../NonLinearSolver/Domain ../NonLinearSolver/BoundaryConditions ../NonLinearSolver/nlTerms ../NonLinearSolver/field ../NonLinearSolver/space ${GMSH_EXTERNAL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/NonLinearSolver/gmsh/Common src ../../contrib/gmm ${CMAKE_CURRENT_BINARY_DIR}/NonLinearSolver ../NonLinearSolver/periodicBC ../NonLinearSolver/restart $ENV{NLSMPIINC} )


# define some variables for the executable
#set(CM3_SLEPC_LIB ${ENV_SLEPC_DIR}/${ENV_PETSC_ARCH}/lib)
#set(CM3_PETSC_LIB ${ENV_PETSC_DIR}/${ENV_PETSC_ARCH}/lib)
set(CM3_PYTHON_PACKAGE ${CMAKE_CURRENT_BINARY_DIR}/NonLinearSolver/gmsh/wrappers)

file(GLOB HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src *.h)

if(HAVE_MPI)
  cmake_force_cxx_compiler(${MPI_COMPILER} "MPI C++ Compiler")
endif(HAVE_MPI)

# Testing
# configure testing support
include(CTest)
if (BUILD_TESTING)
  enable_testing()
  # limit test to 60 sec
  #set(CTEST_TEST_TIMEOUT 10)
  # move the launching script in the binary dir
  configure_file(runtestandclean.py "${PROJECT_BINARY_DIR}/runtestandclean.py" COPYONLY)
endif(BUILD_TESTING)

# macro to execute a test
macro(add_cm3python_test PYFILE ARGN)
  # put absolute path on the files to delete
  foreach(f ${ARGN})
    set(DELF ${DELF} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
  endforeach(f ${ARGN})
  
  # add the current folder name to the test name
  get_filename_component(MYDIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  string(REPLACE " " "_" MYDIR ${MYDIR})
  # launch
  add_test(NAME "${PROJECT_NAME}/${MYDIR}/${PYFILE}" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/runtestandclean.py ${CMAKE_BINARY_DIR}/bin/cm3py ${PYFILE} ${DELF})
endmacro(add_cm3python_test)

# same but MPI launch
macro(add_cm3python_mpi_test NCPU PYFILE ARGN)
	if(NOT ENABLE_MPI)
	  message("You compile without MPI disable the MPI test ${PROJECT_NAME}/${MYDIR}/${PYFILE}")
  else(NOT ENABLE_MPI)
    # put absolute path on the files to delete
    foreach(f ${ARGN})
      set(DELF ${DELF} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
    endforeach(f ${ARGN})
  
    # add the current folder name to the test name
    get_filename_component(MYDIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" MYDIR ${MYDIR})
    # launch
    add_test(NAME "${PROJECT_NAME}/${MYDIR}/${PYFILE}" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/runtestandclean.py "${MPIEXEC} -n ${NCPU} ${CMAKE_BINARY_DIR}/bin/cm3py" ${PYFILE} ${DELF})
   set_tests_properties("${PROJECT_NAME}/${MYDIR}/${PYFILE}" PROPERTIES RUN_SERIAL ON) 
  endif(NOT ENABLE_MPI)  
endmacro(add_cm3python_mpi_test)
# add the different projects
if(ENABLE_DGSHELL)
  set(CM3_PYTHON_PACKAGE ${CM3_PYTHON_PACKAGE}:${CMAKE_CURRENT_BINARY_DIR}/dgshell)
  include_directories(../dgshell/src)
  add_subdirectory(../dgshell "${CMAKE_CURRENT_BINARY_DIR}/dgshell")
endif(ENABLE_DGSHELL)

if(ENABLE_DG3D AND NOT ENABLE_MSCH)# otherwise compiled through msch 
	set(CM3_PYTHON_PACKAGE ${CM3_PYTHON_PACKAGE}:${CMAKE_CURRENT_BINARY_DIR}/dG3D)
  include_directories(../dG3D/src)
  add_subdirectory(../dG3D "${CMAKE_CURRENT_BINARY_DIR}/dG3D")
endif(ENABLE_DG3D AND NOT ENABLE_MSCH)

if(ENABLE_MSCH)
  include_directories(../dG3D/src ../msch/src)
  set(CM3_PYTHON_PACKAGE ${CM3_PYTHON_PACKAGE}:${CMAKE_CURRENT_BINARY_DIR}/msch:${CM3_PYTHON_PACKAGE}:${CMAKE_CURRENT_BINARY_DIR}/dG3D)
  add_subdirectory(../msch "${CMAKE_CURRENT_BINARY_DIR}/msch")
endif(ENABLE_MSCH)

# cpp benchmarks (valgrind debug)
if (BUILD_TESTING)
  if(ENABLE_DG3D OR ENABLE_MSCH)
    add_subdirectory(cppbenchmarks)
  endif(ENABLE_DG3D OR ENABLE_MSCH)
endif (BUILD_TESTING)

# configure the executable
configure_file(cm3py.in "${PROJECT_BINARY_DIR}/bin/cm3py" @ONLY)

