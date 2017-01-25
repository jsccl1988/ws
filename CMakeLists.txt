PROJECT(maplite)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/ ${CMAKE_ROOT}/Modules/   )

FIND_PACKAGE(Sqlite3 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

# Boost
FIND_PACKAGE(Boost 1.49 REQUIRED COMPONENTS 
        regex filesystem system coroutine context program_options thread )

ADD_SUBDIRECTORY(src)


