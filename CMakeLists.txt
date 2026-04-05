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
  # Homebrew Qt 6 is often built for macOS 14+. Homebrew openmotif’s libXm may declare a higher
  # min OS (e.g. 26) — ld then warns when linking; that mismatch is usually safe to ignore unless
  # you ship on very old macOS. To silence: cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=<match libXm>.
  if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS version" FORCE)
  endif()
else()
  string(APPEND GMORPH_C_FLAGS " -ansi -D_DEFAULT_SOURCE")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GMORPH_C_FLAGS}")

# Source tree (X11 include paths come from pkg-config / XQuartz below)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files under src/
set(SOURCES
  src/main.c
  src/smd.c
  src/window.c
  src/gldraw.c
  src/draw.c
  src/busy.c
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
  src/Progress.c
  src/time.c
  src/gmorph_qt_bridge.c
  src/gmorph_qt.cpp
  src/qt/gmorph_icons.qrc
)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Build the executable
add_executable(gmorph2b8 ${SOURCES})

if(APPLE)
  target_compile_definitions(gmorph2b8 PRIVATE _DARWIN_C_SOURCE)
  target_compile_options(gmorph2b8 PRIVATE
    "-U__STRICT_ANSI__"
    "-include" "${CMAKE_CURRENT_SOURCE_DIR}/src/gmorph_apple_libcxx_compat.h")
  # X11.pc + Motif + Mesa can each repeat -lX11 -lGL …; harmless but noisy.
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(gmorph2b8 PRIVATE "LINKER:-no_warn_duplicate_libraries")
  endif()
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# macOS: pkg-config must see X11 toolkit .pc files (Xt, SM, ICE, …).
# Option A: Homebrew — `brew install libx11 libxext libxt libxi libsm libice libxpm`
# Option B: XQuartz — https://www.xquartz.org/ (uses /opt/X11/lib/pkgconfig)
if(APPLE)
  set(_gmorph_pc "$ENV{PKG_CONFIG_PATH}")
  foreach(
    _d
    IN
    ITEMS
      /opt/homebrew/lib/pkgconfig
      /usr/local/lib/pkgconfig
      /opt/X11/lib/pkgconfig
  )
    if(EXISTS "${_d}")
      set(_gmorph_pc "${_d}:${_gmorph_pc}")
    endif()
  endforeach()
  set(ENV{PKG_CONFIG_PATH} "${_gmorph_pc}")
  if(EXISTS "/opt/X11/lib/pkgconfig/x11.pc")
    list(APPEND CMAKE_PREFIX_PATH "/opt/X11")
  endif()
endif()

pkg_check_modules(X11 REQUIRED x11 xext xt xi sm ice)
pkg_check_modules(XPM REQUIRED xpm)

# window.c uses #include "xpm.h" (X11/Xpm); pkg-config xpm often only exposes /usr/include
find_path(GMORPH_X11_XPM_INCLUDE_DIR NAMES xpm.h
  PATH_SUFFIXES X11
  PATHS
    ${X11_INCLUDE_DIRS}
    ${XPM_INCLUDE_DIRS}
    /usr/include
    /opt/X11/include
    /usr/X11/include
    /usr/local/include
)
if(NOT GMORPH_X11_XPM_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find X11/xpm.h (install libxpm / XQuartz).")
endif()

# OpenMotif (Xm) — required: almost all sources include motif.h / Xm headers.
pkg_check_modules(MOTIF motif)
if(NOT MOTIF_FOUND)
  message(STATUS "Motif not found via pkg-config; searching common paths (Homebrew keg, Linux)...")
  find_library(
    MOTIF_LIBRARY
    NAMES Xm libXm
    PATHS
      /opt/homebrew/opt/openmotif/lib
      /usr/local/opt/openmotif/lib
      /opt/homebrew/lib
      /usr/local/lib
      /opt/local/lib
      /usr/lib
      /usr/lib/x86_64-linux-gnu
  )
  find_path(
    MOTIF_INCLUDE_DIR
    NAMES Xm/Xm.h
    PATHS
      /opt/homebrew/opt/openmotif/include
      /usr/local/opt/openmotif/include
      /opt/homebrew/include
      /usr/local/include
      /opt/local/include
      /usr/include
      /usr/include/X11
  )
  if(MOTIF_LIBRARY AND MOTIF_INCLUDE_DIR)
    set(MOTIF_LIBRARIES ${MOTIF_LIBRARY})
    set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
    set(MOTIF_FOUND TRUE)
    message(STATUS "Found Motif library: ${MOTIF_LIBRARY}")
    message(STATUS "Found Motif include directory: ${MOTIF_INCLUDE_DIR}")
  endif()
endif()

# Homebrew: `brew --prefix openmotif` (headers often only under opt/openmotif/include)
if(NOT MOTIF_FOUND)
  find_program(GMORPH_BREW_EXECUTABLE brew PATHS /opt/homebrew/bin /usr/local/bin)
  if(GMORPH_BREW_EXECUTABLE)
    execute_process(
      COMMAND ${GMORPH_BREW_EXECUTABLE} --prefix openmotif
      OUTPUT_VARIABLE GMORPH_OPENMOTIF_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _om_res
    )
    if(_om_res EQUAL 0 AND GMORPH_OPENMOTIF_PREFIX)
      find_library(
        MOTIF_LIBRARY
        NAMES Xm libXm
        PATHS "${GMORPH_OPENMOTIF_PREFIX}/lib"
        NO_DEFAULT_PATH
      )
      find_path(
        MOTIF_INCLUDE_DIR
        NAMES Xm/Xm.h
        PATHS "${GMORPH_OPENMOTIF_PREFIX}/include"
        NO_DEFAULT_PATH
      )
      if(MOTIF_LIBRARY AND MOTIF_INCLUDE_DIR)
        set(MOTIF_LIBRARIES ${MOTIF_LIBRARY})
        set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
        set(MOTIF_FOUND TRUE)
        message(STATUS "Found Motif via Homebrew prefix: ${GMORPH_OPENMOTIF_PREFIX}")
      endif()
    endif()
  endif()
endif()

if(NOT MOTIF_FOUND)
  message(
    FATAL_ERROR
      "OpenMotif (Xm/Xm.h + libXm) is required to build this tree.\n"
      "  macOS:  brew install openmotif\n"
      "          (headers live in e.g. /opt/homebrew/opt/openmotif/include, not always in /opt/homebrew/include)\n"
      "  Debian/Ubuntu: sudo apt install libmotif-dev\n"
      "Qt (-gui) still uses the same sources; you cannot omit OpenMotif without a source refactor.")
endif()

# Find OpenGL
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# GLw: Silicon Graphics OpenGL widget for Motif. Homebrew X11 (libx11, libxt, …) does not
# ship libGLw; XQuartz provides it at /opt/X11/lib. When missing on macOS we link stubs so
# Qt (-gui) builds still work. Linux: install libglw / mesa GLw, or -DGMORPH_USE_GLW_STUB=ON.
option(GMORPH_USE_GLW_STUB
  "Force stub GLw (Qt -gui only; Motif GL canvas unavailable)."
  OFF)

find_path(GMORPH_GLW_INCLUDE_ROOT GL/GLwMDrawA.h
  PATHS
    /opt/X11/include
    /opt/homebrew/include
    /usr/include
    /usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glw-sgi
)
if(NOT GMORPH_GLW_INCLUDE_ROOT)
  message(FATAL_ERROR
    "GL/GLwMDrawA.h not found. Add headers under third_party/glw-sgi/GL/ or install XQuartz / mesa GLw dev package.")
endif()

find_library(GLW_LIBRARY NAMES GLw libGLw
  PATHS
    ${X11_LIBRARY_DIRS}
    ${XPM_LIBRARY_DIRS}
    /opt/X11/lib
    /opt/homebrew/lib
    /usr/local/lib
    /usr/X11/lib
    /usr/lib
    /usr/lib/x86_64-linux-gnu
)

set(GMORPH_GLW_STUB FALSE)
if(GMORPH_USE_GLW_STUB)
  set(GMORPH_GLW_STUB TRUE)
  message(STATUS "GLw: using stub (GMORPH_USE_GLW_STUB=ON)")
elseif(NOT GLW_LIBRARY AND APPLE)
  set(GMORPH_GLW_STUB TRUE)
  message(WARNING
    "libGLw not found (typical on macOS without XQuartz). Using GLw stub — use -gui for Qt, "
    "or install XQuartz for Motif + GLw. https://www.xquartz.org/")
elseif(NOT GLW_LIBRARY)
  message(FATAL_ERROR
    "GLw library not found. Install your distro's GLw package (e.g. libglw1-mesa-dev), "
    "or configure with -DGMORPH_USE_GLW_STUB=ON if you only need Qt (-gui).")
endif()

if(GMORPH_GLW_STUB)
  target_sources(gmorph2b8 PRIVATE src/gmorph_glw_stub.c)
endif()

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

# macOS: OpenGL.framework (CMake OpenGL::GL) has no GLX — Motif/X11 code needs glX* from Mesa or XQuartz libGL.
set(GMORPH_APPLE_GLX_LIBS "")
set(GMORPH_APPLE_GLX_LIBDIRS "")
if(APPLE)
  pkg_check_modules(_GMORPH_GLX_GL QUIET gl)
  if(_GMORPH_GLX_GL_FOUND)
    set(GMORPH_APPLE_GLX_LIBS ${_GMORPH_GLX_GL_LIBRARIES})
    set(GMORPH_APPLE_GLX_LIBDIRS ${_GMORPH_GLX_GL_LIBRARY_DIRS})
    message(STATUS "GLX libGL (pkg-config gl): ${GMORPH_APPLE_GLX_LIBS}")
  else()
    find_library(
      _GMORPH_MESA_GL
      NAMES GL libGL
      PATHS
        /opt/homebrew/opt/mesa/lib
        /opt/homebrew/lib
        /opt/X11/lib
        /usr/local/opt/mesa/lib
        /usr/local/lib
    )
    if(_GMORPH_MESA_GL)
      set(GMORPH_APPLE_GLX_LIBS ${_GMORPH_MESA_GL})
      get_filename_component(_gmorph_glx_dir "${_GMORPH_MESA_GL}" DIRECTORY)
      set(GMORPH_APPLE_GLX_LIBDIRS "${_gmorph_glx_dir}")
      message(STATUS "GLX libGL: ${_GMORPH_MESA_GL}")
    endif()
  endif()
  if(NOT GMORPH_APPLE_GLX_LIBS)
    message(
      FATAL_ERROR
        "macOS: could not find libGL with GLX (glX* symbols). CMake's OpenGL.framework does not provide GLX.\n"
        "  Install Mesa:  brew install mesa\n"
        "  or use XQuartz so /opt/X11/lib/libGL.dylib exists.")
  endif()
endif()

# Link libraries
set(GMORPH_LINK_LIBS
  Qt6::Widgets
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  $<$<BOOL:${MOTIF_FOUND}>:${MOTIF_LIBRARIES}>
  ${X11_LIBRARIES}
  ${XPM_LIBRARIES}
  OpenGL::GL
  OpenGL::GLU
  ${GMORPH_APPLE_GLX_LIBS}
)
if(NOT GMORPH_GLW_STUB)
  list(APPEND GMORPH_LINK_LIBS ${GLW_LIBRARY})
endif()

# libm is folded into libSystem on Apple platforms
if(NOT APPLE)
  list(APPEND GMORPH_LINK_LIBS m)
endif()

target_link_libraries(gmorph2b8 PRIVATE ${GMORPH_LINK_LIBS})

# Add target include directories
target_include_directories(gmorph2b8 PRIVATE
  ${X11_INCLUDE_DIRS}
  ${XPM_INCLUDE_DIRS}
  ${GMORPH_X11_XPM_INCLUDE_DIR}
  ${GMORPH_GLW_INCLUDE_ROOT}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${EIGEN3_INCLUDE_DIR}
)

if(MOTIF_FOUND)
  target_include_directories(gmorph2b8 PRIVATE ${MOTIF_INCLUDE_DIRS})
  if(MOTIF_INCLUDE_DIR)
    target_include_directories(gmorph2b8 PRIVATE ${MOTIF_INCLUDE_DIR})
  endif()
endif()

# Link directories from pkg-config (avoid hard-coded Linux-only paths on macOS)
target_link_directories(gmorph2b8 PRIVATE
  ${X11_LIBRARY_DIRS}
  ${XPM_LIBRARY_DIRS}
)

if(NOT APPLE)
  target_link_directories(gmorph2b8 PRIVATE /usr/lib /usr/X11/lib)
elseif(EXISTS "/opt/X11/lib")
  target_link_directories(gmorph2b8 PRIVATE /opt/X11/lib)
endif()

if(APPLE AND GMORPH_APPLE_GLX_LIBDIRS)
  target_link_directories(gmorph2b8 PRIVATE ${GMORPH_APPLE_GLX_LIBDIRS})
endif()

if(MOTIF_FOUND)
  target_link_directories(gmorph2b8 PRIVATE ${MOTIF_LIBRARY_DIRS})
endif()

target_link_options(gmorph2b8 PRIVATE
  ${X11_LDFLAGS_OTHER}
  ${XPM_LDFLAGS_OTHER}
)

if(MOTIF_FOUND)
  target_link_options(gmorph2b8 PRIVATE ${MOTIF_LDFLAGS_OTHER})
endif()

# Copy the data directory after build (optional)
# add_custom_command(TARGET gmorph2b8 POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/data.v2.0
#     ${CMAKE_BINARY_DIR}/data
# )
