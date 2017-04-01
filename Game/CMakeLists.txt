PROJECT(Game)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(../dependencies.cmake)

SET(PROJECT_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../Base/include
  ${PROJECT_SOURCE_DIR}/../Map/include
  ${PROJECT_SOURCE_DIR}/../argon2/include
  ${PROJECT_SOURCE_DIR}/../tdr/AGL/include
)

SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(Game_library_SRCS
  ${PROJECT_SOURCE_DIR}/main.cpp
  ${PROJECT_SOURCE_DIR}/Config.cpp
  ${PROJECT_SOURCE_DIR}/Server.cpp
  ${PROJECT_SOURCE_DIR}/Client.cpp
  ${PROJECT_SOURCE_DIR}/GameState.cpp
  ${PROJECT_SOURCE_DIR}/Player.cpp
  ${PROJECT_SOURCE_DIR}/Database.cpp
  ${PROJECT_SOURCE_DIR}/sha2.c
  ${PROJECT_SOURCE_DIR}/Credentials.cpp
  ${PROJECT_SOURCE_DIR}/packet.cpp
  ${PROJECT_SOURCE_DIR}/Location.cpp
  ${PROJECT_SOURCE_DIR}/window/ClientWindow.cpp
  ${PROJECT_SOURCE_DIR}/window/ChatWindow.cpp
  ${PROJECT_SOURCE_DIR}/window/TerrainRenderer.cpp
  ${PROJECT_SOURCE_DIR}/window/Patcher.cpp
  ${PROJECT_SOURCE_DIR}/window/patcher_views/TextureView.cpp
  ${PROJECT_SOURCE_DIR}/window/patcher_views/ModelView.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
# actual target:
ADD_LIBRARY(Game ${Game_library_SRCS})

#ADD_CUSTOM_COMMAND(
#  TARGET AGLTest
#  POST_BUILD
#  COMMAND cp -r shader textures ../build
#)

# add install target:
INSTALL(TARGETS Game DESTINATION lib)
