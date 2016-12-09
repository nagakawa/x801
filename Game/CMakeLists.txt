PROJECT(Game)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(../dependencies.cmake)

SET(PROJECT_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../Base/include
  ${PROJECT_SOURCE_DIR}/../Map/include
)

SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(Game_executable_SRCS
  ${PROJECT_SOURCE_DIR}/Main.cpp
  ${PROJECT_SOURCE_DIR}/Config.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
# actual target:
ADD_EXECUTABLE(Game ${Game_executable_SRCS})

TARGET_LINK_LIBRARIES(Game
  ${PROJECT_SOURCE_DIR}/../../build/libMap.a
  ${PROJECT_SOURCE_DIR}/../../build/libBase.a
  ${ZLIB_LIBRARIES}
)

#ADD_CUSTOM_COMMAND(
#  TARGET AGLTest
#  POST_BUILD
#  COMMAND cp -r shader textures ../build
#)

# add install target:
INSTALL(TARGETS Game DESTINATION bin)
