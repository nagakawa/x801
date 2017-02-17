
FIND_PACKAGE(OpenGL)

if (OPENGL_FOUND)
  INCLUDE_DIRECTORIES("${OPENGL_INCLUDE_DIR}")
else()
  MESSAGE(FATAL_ERROR "Could not find OpenGL")
endif()

FIND_PACKAGE(GLEW)

if (GLEW_FOUND)
  INCLUDE_DIRECTORIES("${GLEW_INCLUDE_DIR}")
else()
  MESSAGE(FATAL_ERROR "Could not find GLEW")
endif()

#FIND_PACKAGE(GTK2)

#if (GTK2_FOUND)
#  INCLUDE_DIRECTORIES("${GTK2_INCLUDE_DIR}")
#else()
#  MESSAGE(FATAL_ERROR "Could not find GTK2")
#endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules/")
FIND_PACKAGE(Pango REQUIRED)
INCLUDE_DIRECTORIES(${Pango_1_0_INCLUDE_DIR})

FIND_PACKAGE(Cairo REQUIRED)
INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIRS})

FIND_PACKAGE(PangoCairo REQUIRED)
INCLUDE_DIRECTORIES(${PangoCairo_INCLUDE_DIRS})

FIND_PACKAGE(GLM REQUIRED)
INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIRS})

FIND_PACKAGE(GLFW REQUIRED)
INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIRS})

FIND_PACKAGE(SOIL REQUIRED)
INCLUDE_DIRECTORIES(${SOIL_INCLUDE_DIR})

FIND_PACKAGE(GLib REQUIRED)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})

FIND_PACKAGE(GObject REQUIRED)
INCLUDE_DIRECTORIES(${GOBJECT_INCLUDE_DIR})

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

# OPTION(Boost_NO_BOOST_CMAKE "" ON)
OPTION(Boost_DEBUG "" OFF)
FIND_PACKAGE(BoostCustom REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Will have to make this work with other platforms.

IF (NOT Boost_LIBRARIES)
  MESSAGE(STATUS "Didn't find the library files.")
  MESSAGE(STATUS "Setting them to defaults.")
  FILE(
    GLOB Boost_LIBRARIES
    /usr/lib/x86_64-linux-gnu/libboost_*.so.*.*.*
  )
ENDIF()

FIND_PACKAGE(LibPThread REQUIRED)

FIND_PACKAGE(SQLite REQUIRED)

FIND_PACKAGE(PortAudio REQUIRED)
INCLUDE_DIRECTORIES(${PORTAUDIO_INCLUDE_DIRS})

FIND_PACKAGE(Vorbis REQUIRED)
INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${SQLITE_INCLUDE_DIR})

# RakNet
INCLUDE_DIRECTORIES(SYSTEM ${PROJECT_SOURCE_DIR}/../RakNet/Source)
INCLUDE_DIRECTORIES(SYSTEM ${PROJECT_SOURCE_DIR}/../RakNet/DependentExtensions)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../tdr/AGL)

# dear imgui

INCLUDE_DIRECTORIES(SYSTEM ${PROJECT_SOURCE_DIR}/../imgui)
INCLUDE_DIRECTORIES(SYSTEM ${PROJECT_SOURCE_DIR}/../imgui/examples/opengl3_example)

# SET(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../FileUtil)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

# Was going to pass in -Weffc++ but it blindly tells you to initialise fields in an
# initialiser list, even when it's more readable to initialise it in the constructor
# body.
SET(COMMON_FLAGS "-std=c++14 -Wall -Wpedantic -Wextra -Werror -fno-builtin")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_FLAGS} -O2 -g")
#SET(CMAKE_C_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -ffat-lto-objects -flto")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_FLAGS} -O3 -ffat-lto-objects -flto")
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} -ffat-lto-objects -flto -Wno-error")
SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
