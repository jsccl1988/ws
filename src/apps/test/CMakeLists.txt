SET ( TEST_APP_SOURCES

        ${SRC_ROOT}/util/dictionary.cpp
        ${SRC_ROOT}/util/dictionary.hpp

        ${SRC_ROOT}/server/request_handler.hpp

        plugin.cpp
)

ADD_LIBRARY(ws_test_plugin SHARED ${TEST_APP_SOURCES})
TARGET_LINK_LIBRARIES(ws_test_plugin ${Boost_LIBRARIES} ws_http_server)



