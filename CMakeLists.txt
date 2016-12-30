# ########## Project setup ##########
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
PROJECT(x801 VERSION 0.0)

IF(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/RakNet/README.md)
  # Get our fork of RakNet
  EXECUTE_PROCESS(
    COMMAND git submodule update --init --recursive
  )
ELSE()
  MESSAGE(
    STATUS
    "If the build fails, try running "
    "`git submodule update --init --recursive`."
  )
ENDIF()

# ######### General setup ##########
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

# External dependency managed by this script

# Samples don't all build on GNU / Linux for some odd reason
# so not buliding that.
OPTION(RAKNET_ENABLE_SAMPLES "" FALSE)
OPTION(RAKNET_GENERATE_INCLUDE_ONLY_DIR "" TRUE)
ADD_SUBDIRECTORY(RakNet)

# Our own submodules

ADD_SUBDIRECTORY(Base)
ADD_SUBDIRECTORY(Map)
ADD_SUBDIRECTORY(Test)
ADD_SUBDIRECTORY(Game)
ADD_SUBDIRECTORY(x801)

# actual target:
# ADD_LIBRARY(tdr ${tdr_library}_SRCS ${tdr_library_HDRS})
# SET_TARGET_PROPERTIES(tdr PROPERTIES VERSION 0.0.0)

# add install target:
# INSTALL(TARGETS tdr DESTINATION lib)
