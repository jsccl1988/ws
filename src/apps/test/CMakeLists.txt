
INCLUDE(${CMAKE_SOURCE_DIR}/CMake/Templates.cmake)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_EXECUTABLE(test_templates test_templates.cpp)
                # TARGET        PREFIX    SOURCE
ADD_WS_TEMPLATES(test_templates test_app templates/test.tpl templates/xx.tpl)
TARGET_LINK_LIBRARIES(test_templates ${Boost_LIBRARIES} wspp_http_server wspp_util)



