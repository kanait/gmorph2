cmake_minimum_required(VERSION 3.10)
project(gmorph2b8 C CXX)

# Set language standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
  set(CMAKE_CXX_EXTENSIONS ON)
endif()

# --- Platform-specific C flags ---
set(GMORPH_C_FLAGS "-O -g3 -funroll-loops -pedantic -ffast-math -DSHM")
if(APPLE)
  # macOS / Clang: no glibc _DEFAULT_SOURCE; -ansi is overly strict with system headers.
  string(APPEND GMORPH_C_FLAGS " -D_DARWIN_C_SOURCE")
  # Homebrew Qt 6 is often built for macOS 14+.
  if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS version" FORCE)
  endif()
else()
  string(APPEND GMORPH_C_FLAGS " -ansi -D_DEFAULT_SOURCE")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GMORPH_C_FLAGS}")

# Qt6 GUI + batch CLI. OpenMotif / X11 / GLw UI has been removed from the build.
option(GMORPH_BUILD_CLI_ONLY_TARGET
  "Build an additional CLI-only executable (no Qt, no OpenGL)."
  ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files under src/
set(SOURCES
  src/main.c
  src/smd.c
  src/gldraw.c
  src/draw.c
  src/pick.c
  src/edit.c
  src/sellist.c
  src/file.c
  src/screen.c
  src/ppd.c
  src/ppdsolid.c
  src/ppdpart.c
  src/ppdface.c
  src/ppdloop.c
  src/ppdvertex.c
  src/ppdnormal.c
  src/ppdedge.c
  src/ppdlist.c
  src/oppd.c
  src/prique.c
  src/sgprique.c
  src/quadtree.c
  src/loop.c
  src/gmorph.c
  src/grouping.c
  src/harmonic.c
  src/harmonic_eigen.cpp
  src/correspond.c
  src/morphvec.c
  src/paths.c
  src/sgraph.c
  src/sgpaths.c
  src/hppd.c
  src/hppdlod.c
  src/hgppd.c
  src/hged.c
  src/glppd.c
  src/ps.c
  src/image.c
  src/sgiimage.c
  src/trisquare.c
  src/matlib.c
  src/veclib.c
  src/gmorph_view_ops.c
  src/gmorph_displayinfo.c
  src/time.c
  src/gmorph_qt_bridge.c
  src/gmorph_qt.cpp
  src/qt/gmorph_icons.qrc
  src/progress_stub.c
  src/busy_stub.c
)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Build the executable
add_executable(gmorph2b8 ${SOURCES})

target_compile_definitions(gmorph2b8 PRIVATE GMORPH_ENABLE_RENDERING=1)

if(APPLE)
  target_compile_definitions(gmorph2b8 PRIVATE _DARWIN_C_SOURCE)
  target_compile_options(gmorph2b8 PRIVATE
    "-U__STRICT_ANSI__"
    "-include" "${CMAKE_CURRENT_SOURCE_DIR}/src/gmorph_apple_libcxx_compat.h")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(gmorph2b8 PRIVATE "LINKER:-no_warn_duplicate_libraries")
  endif()
endif()

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# Eigen (harmonic map sparse solver)
find_path(EIGEN3_INCLUDE_DIR NAMES Eigen/Core
  PATHS
    /opt/homebrew/include/eigen3
    /usr/local/include/eigen3
    /usr/include/eigen3
)
if(NOT EIGEN3_INCLUDE_DIR)
  message(FATAL_ERROR
    "Eigen3 headers not found. Install Eigen (e.g. Ubuntu: libeigen3-dev; macOS: brew install eigen).")
endif()
message(STATUS "Found Eigen3 include directory: ${EIGEN3_INCLUDE_DIR}")

target_link_libraries(gmorph2b8 PRIVATE
  Qt6::Widgets
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  OpenGL::GL
  OpenGL::GLU
)
if(NOT APPLE)
  target_link_libraries(gmorph2b8 PRIVATE m)
endif()

# --- CLI-only executable (no Qt, no OpenGL) ---
if(GMORPH_BUILD_CLI_ONLY_TARGET)
  set(CLI_SOURCES
    src/main.c
    src/smd.c
    src/sellist.c
    src/file.c
    src/ppd.c
    src/ppdsolid.c
    src/ppdpart.c
    src/ppdface.c
    src/ppdloop.c
    src/ppdvertex.c
    src/ppdnormal.c
    src/ppdedge.c
    src/ppdlist.c
    src/oppd.c
    src/prique.c
    src/sgprique.c
    src/quadtree.c
    src/loop.c
    src/gmorph.c
    src/grouping.c
    src/harmonic.c
    src/harmonic_eigen.cpp
    src/correspond.c
    src/morphvec.c
    src/paths.c
    src/sgraph.c
    src/sgpaths.c
    src/hppd.c
    src/hppdlod.c
    src/hgppd.c
    src/hged.c
    src/ps.c
    src/sgiimage.c
    src/trisquare.c
    src/matlib.c
    src/veclib.c
    src/gmorph_view_ops.c
    src/gmorph_displayinfo.c
    src/time.c
    src/progress_stub.c
    src/busy_stub.c
    src/cli_stubs.c
  )

  add_executable(gmorph2b8_cli ${CLI_SOURCES})
  target_compile_definitions(gmorph2b8_cli PRIVATE GMORPH_ENABLE_RENDERING=0)
  target_include_directories(gmorph2b8_cli PRIVATE ${EIGEN3_INCLUDE_DIR})
  if(APPLE)
    target_compile_definitions(gmorph2b8_cli PRIVATE _DARWIN_C_SOURCE)
    target_compile_options(gmorph2b8_cli PRIVATE
      "-U__STRICT_ANSI__"
      "-include" "${CMAKE_CURRENT_SOURCE_DIR}/src/gmorph_apple_libcxx_compat.h")
  endif()
endif()

target_include_directories(gmorph2b8 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${EIGEN3_INCLUDE_DIR}
)

# Copy the data directory after build (optional)
# add_custom_command(TARGET gmorph2b8 POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/data.v2.0
#     ${CMAKE_BINARY_DIR}/data
# )
