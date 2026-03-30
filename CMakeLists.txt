cmake_minimum_required(VERSION 3.10)
project(gmorph2b8 C)

# C標準を設定
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# コンパイラフラグを設定
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O -g3 -funroll-loops -ansi -pedantic -ffast-math -D_DEFAULT_SOURCE -DSHM")

# インクルードディレクトリを設定
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src /usr/include/X11)

# ソースファイルのリスト（src/ 配下）
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
    src/linbcg.c
    src/trisquare.c
    src/matlib.c
    src/veclib.c
    src/Progress.c
    src/time.c
)

# 実行ファイルを作成
add_executable(gmorph2b8 ${SOURCES})

# 必要なライブラリを検索
find_package(PkgConfig REQUIRED)
pkg_check_modules(X11 REQUIRED x11 xext xt xi sm ice)
pkg_check_modules(XPM REQUIRED xpm)

# Motifライブラリを検索（オプション）
pkg_check_modules(MOTIF motif)
if(NOT MOTIF_FOUND)
    message(WARNING "Motif library not found via pkg-config. Trying to find it manually...")
    find_library(MOTIF_LIBRARY NAMES Xm libXm)
    find_path(MOTIF_INCLUDE_DIR NAMES Xm/Xm.h)
    if(MOTIF_LIBRARY AND MOTIF_INCLUDE_DIR)
        set(MOTIF_LIBRARIES ${MOTIF_LIBRARY})
        set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
        set(MOTIF_FOUND TRUE)
        message(STATUS "Found Motif library: ${MOTIF_LIBRARY}")
        message(STATUS "Found Motif include directory: ${MOTIF_INCLUDE_DIR}")
    else()
        # より詳細な検索を試行
        find_library(MOTIF_LIBRARY 
            NAMES Xm libXm
            PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
        )
        find_path(MOTIF_INCLUDE_DIR 
            NAMES Xm/Xm.h
            PATHS /usr/include /usr/include/X11 /usr/local/include
        )
        if(MOTIF_LIBRARY AND MOTIF_INCLUDE_DIR)
            set(MOTIF_LIBRARIES ${MOTIF_LIBRARY})
            set(MOTIF_INCLUDE_DIRS ${MOTIF_INCLUDE_DIR})
            set(MOTIF_FOUND TRUE)
            message(STATUS "Found Motif library (manual search): ${MOTIF_LIBRARY}")
            message(STATUS "Found Motif include directory (manual search): ${MOTIF_INCLUDE_DIR}")
        else()
            message(WARNING "Motif library not found. You may need to install it:")
            message(WARNING "  Ubuntu/Debian: sudo apt-get install libmotif-dev")
            message(WARNING "  CentOS/RHEL: sudo yum install motif-devel")
            message(WARNING "  Or disable Motif support if not needed")
            message(WARNING "  Library search paths tried: /usr/lib, /usr/lib/x86_64-linux-gnu, /usr/local/lib")
            message(WARNING "  Include search paths tried: /usr/include, /usr/include/X11, /usr/local/include")
        endif()
    endif()
endif()

# OpenGLライブラリを検索
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# ライブラリをリンク
# Motif を Xt/X11 より前にリンク（NEEDED 順が後だと起動時 BadWindow になることがある）
target_link_libraries(gmorph2b8
    $<$<BOOL:${MOTIF_FOUND}>:${MOTIF_LIBRARIES}>
    ${X11_LIBRARIES}
    ${XPM_LIBRARIES}
    OpenGL::GL
    OpenGL::GLU
    GLw
    m
)

# インクルードディレクトリを追加
target_include_directories(gmorph2b8 PRIVATE
    ${X11_INCLUDE_DIRS}
    ${XPM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Motifライブラリが利用可能な場合のみインクルードディレクトリを追加
if(MOTIF_FOUND)
    target_include_directories(gmorph2b8 PRIVATE ${MOTIF_INCLUDE_DIRS})
    # 手動検索で見つかった場合のインクルードディレクトリも追加
    if(MOTIF_INCLUDE_DIR)
        target_include_directories(gmorph2b8 PRIVATE ${MOTIF_INCLUDE_DIR})
    endif()
endif()

# リンクディレクトリを追加
target_link_directories(gmorph2b8 PRIVATE
    ${X11_LIBRARY_DIRS}
    ${XPM_LIBRARY_DIRS}
    /usr/lib
    /usr/X11/lib
)

# Motifライブラリが利用可能な場合のみリンクディレクトリを追加
if(MOTIF_FOUND)
    target_link_directories(gmorph2b8 PRIVATE ${MOTIF_LIBRARY_DIRS})
endif()

# リンクフラグを追加
target_link_options(gmorph2b8 PRIVATE
    ${X11_LDFLAGS_OTHER}
    ${XPM_LDFLAGS_OTHER}
)

# Motifライブラリが利用可能な場合のみリンクフラグを追加
if(MOTIF_FOUND)
    target_link_options(gmorph2b8 PRIVATE ${MOTIF_LDFLAGS_OTHER})
endif()

# データディレクトリをコピー（オプション）
# add_custom_command(TARGET gmorph2b8 POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/data.v2.0
#     ${CMAKE_BINARY_DIR}/data
# )
