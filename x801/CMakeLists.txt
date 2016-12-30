PROJECT(x801)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(../dependencies.cmake)

SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(x801_executable_SRCS
  ${PROJECT_SOURCE_DIR}/main.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
# actual target:
ADD_EXECUTABLE(x801 ${x801_executable_SRCS})

TARGET_LINK_LIBRARIES(x801
  ${PROJECT_SOURCE_DIR}/../build/libGame.a
  ${PROJECT_SOURCE_DIR}/../build/libMap.a
  ${PROJECT_SOURCE_DIR}/../build/libBase.a
  ${PROJECT_SOURCE_DIR}/../build/libRakNetLibStatic.a
  ${ZLIB_LIBRARIES}
  ${LIBPTHREAD_LIBRARIES}
  ${Boost_LIBRARIES}
  ${SQLITE_LIBRARIES}
)

#ADD_CUSTOM_COMMAND(
#  TARGET AGLTest
#  POST_BUILD
#  COMMAND cp -r shader textures ../build
#)

# add install target:
INSTALL(TARGETS x801 DESTINATION bin)
