CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(REDEX)

GET_PROPERTY(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

IF ((NOT(DEFINED BITVALUE) AND ("${LIB64}" STREQUAL "TRUE") OR ("${BITVALUE}" STREQUAL "64")))
    SET(LIBSUFFIX "_x64")
    SET(BITVALUE 64)
ELSE()
    SET(LIBSUFFIX "")
    SET(BITVALUE 32)
ENDIF()

FILE(GLOB LIBSOURCES "Source/*.cpp" "Source/utils/*.cpp" "Source/mysql/*.cpp" "Source/mysql/datacache/*.cpp" "Source/fileio/*.cpp" "Source/datetime/*.cpp" "Source/randomlist/*.cpp" "Source/rest/*.cpp")
FILE(GLOB TESTSSOURCES "Source/TestApps/*.cpp")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/Binaries)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} -s -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} -s -DDEBUG")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
string(TIMESTAMP TODAY "%Y%m%d%H%m")
ADD_DEFINITIONS(-DBUILD_TIME=${TODAY})

IF (WIN32)
SET(MYSQL_INCLUDE_DIR C:/MinGW/${BITVALUE}bit/include/mysql)
SET(MYSQL_LIBRARY_DIR C:/MinGW/${BITVALUE}bit/lib)
SET(MYSQL_LIBRARY ${MYSQL_LIBRARY_DIR}/libmysql.lib)
SET(MYSQL_LIBRARY ssl crypto crypt32 z mysqlclient ws2_32 gdi32 secur32)
ELSE(WIN32)
SET(MYSQL_INCLUDE_DIR /usr/include/mysql)
SET(MYSQL_LIBRARY /usr/lib${BITVALUE})
SET(MYSQL_LIBRARY ssl dl z crypto mysqlclient)

FIND_PACKAGE(Threads REQUIRED)
ENDIF(WIN32)

INCLUDE_DIRECTORIES("Source")
INCLUDE_DIRECTORIES("${MYSQL_INCLUDE_DIR}")

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME ON)

#FIND_PACKAGE(Boost
#             1.54.0
#             COMPONENTS thread
#                        system
#                        random
#                        regex
#                        filesystem
#             REQUIRED)
#
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

#set(FETCHCONTENT_SOURCE_DIR_BOOST /home/lego/DesolationREDUX/Arma_3_Extensions/libredex/boost-src)
add_subdirectory(boost-cmake)

message(STATUS "${MYSQL_LIBRARY}")
message(STATUS "${Boost_LIBRARIES}")
message(STATUS "${LIBSUFFIX}")
message(STATUS "${BITVALUE}")

IF (WIN32)
    SET(SYSTEMLIBS rpcrt4 wsock32 ws2_32)
    IF(MINGW)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${BITVALUE} -static -Wno-attributes")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${BITVALUE} -static -Wno-attributes")
    ENDIF(MINGW)
ELSE(WIN32)
    SET(SYSTEMLIBS uuid)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${BITVALUE} -static-libgcc -g")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${BITVALUE} -static-libgcc -static-libstdc++ -g")
ENDIF(WIN32)

IF (WIN32)
    ADD_LIBRARY(redex${LIBSUFFIX} SHARED ${LIBSOURCES} "Source/redex.rc")
ELSE(WIN32)
    ADD_LIBRARY(redex${LIBSUFFIX} SHARED ${LIBSOURCES})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(redex${LIBSUFFIX} ${SYSTEMLIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${MYSQL_LIBRARY} Boost::filesystem Boost::thread Boost::system Boost::random Boost::regex)

IF (WIN32)
    SET_TARGET_PROPERTIES(redex${LIBSUFFIX} PROPERTIES LINK_SEARCH_END_STATIC 1)
ENDIF()

#FOREACH( TESTSSOURCEFILE ${TESTSSOURCES} )
#    GET_FILENAME_COMPONENT(TESTNAME ${TESTSSOURCEFILE} NAME)
#    STRING( REPLACE ".cpp" "" TESTNAME ${TESTNAME} )
#    ADD_EXECUTABLE( ${TESTNAME} ${TESTSSOURCEFILE} Source/rest/handle_callback.hpp Source/rest/handle_callback.cpp)
#    TARGET_LINK_LIBRARIES( ${TESTNAME} redex${LIBSUFFIX} ${SYSTEMLIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${MYSQL_LIBRARY} Boost::filesystem Boost::thread Boost::system Boost::random Boost::regex)
#ENDFOREACH( TESTSSOURCEFILE ${TESTSSOURCES} )
