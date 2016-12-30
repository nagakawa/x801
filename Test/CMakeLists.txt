PROJECT(Test)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(../dependencies.cmake)

SET(PROJECT_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../Base/include
  ${PROJECT_SOURCE_DIR}/../Map/include
  ${PROJECT_SOURCE_DIR}/../Game/include
)

SET(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(Test_executable_SRCS
  ${PROJECT_SOURCE_DIR}/Test.cpp
  ${PROJECT_SOURCE_DIR}/TestAux.cpp
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
# actual target:
ADD_EXECUTABLE(Test ${Test_executable_SRCS})

ADD_DEPENDENCIES(Test Argon2)

TARGET_LINK_LIBRARIES(Test
  ${PROJECT_SOURCE_DIR}/../../build/libGame.a
  ${PROJECT_SOURCE_DIR}/../../build/libMap.a
  ${PROJECT_SOURCE_DIR}/../../build/libBase.a
  ${PROJECT_SOURCE_DIR}/../../argon2/libargon2.a
  ${ZLIB_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${Boost_LIBRARIES}
)

#ADD_CUSTOM_COMMAND(
#  TARGET AGLTest
#  POST_BUILD
#  COMMAND cp -r shader textures ../build
#)

# add install target:
INSTALL(TARGETS Test DESTINATION bin)
