SET( UTIL_SOURCES

    ${SRC_ROOT}/util/dictionary.cpp
    ${SRC_ROOT}/util/logger.cpp
    ${SRC_ROOT}/util/database.cpp
    ${SRC_ROOT}/util/crypto.cpp

    ${INCLUDE_ROOT}/util/dictionary.hpp
    ${INCLUDE_ROOT}/util/logger.hpp
    ${INCLUDE_ROOT}/util/database.hpp
    ${INCLUDE_ROOT}/util/crypto.hpp
)

SET ( VIEWS_SOURCES
    ${SRC_ROOT}/views/renderer.cpp
    ${SRC_ROOT}/views/forms.cpp
    ${SRC_ROOT}/views/table.cpp
    ${SRC_ROOT}/views/menu.cpp

    ${INCLUDE_ROOT}/views/renderer.hpp
    ${INCLUDE_ROOT}/views/forms.hpp
    ${INCLUDE_ROOT}/views/table.hpp
    ${INCLUDE_ROOT}/views/menu.hpp
)

SET ( CONTROLLERS_SOURCES
    ${SRC_ROOT}/controllers/user_controller.cpp

    ${INCLUDE_ROOT}/controllers/user_controller.hpp
)

SET ( SERVER_SOURCES
    ${INCLUDE_ROOT}/server/detail/connection.hpp
    ${INCLUDE_ROOT}/server/detail/connection_manager.hpp
    ${INCLUDE_ROOT}/server/detail/io_service_pool.hpp
    ${INCLUDE_ROOT}/server/response.hpp
    ${INCLUDE_ROOT}/server/request_handler.hpp
    ${INCLUDE_ROOT}/server/request.hpp
    ${INCLUDE_ROOT}/server/detail/request_parser.hpp
    ${INCLUDE_ROOT}/server/server.hpp
    ${INCLUDE_ROOT}/server/detail/http_parser.h
    ${INCLUDE_ROOT}/server/session_handler.hpp
    ${INCLUDE_ROOT}/server/fs_session_handler.hpp
    ${INCLUDE_ROOT}/server/session.hpp

    ${SRC_ROOT}/server/connection_manager.cpp
    ${SRC_ROOT}/server/io_service_pool.cpp

    ${SRC_ROOT}/server/response.cpp
    ${SRC_ROOT}/server/request.cpp
    ${SRC_ROOT}/server/request_parser.cpp
    ${SRC_ROOT}/server/server.cpp
    ${SRC_ROOT}/server/http_parser.c
    ${SRC_ROOT}/server/session_handler.cpp
    ${SRC_ROOT}/server/fs_session_handler.cpp
    ${SRC_ROOT}/server/session.cpp
)

ADD_LIBRARY(wspp_util SHARED ${UTIL_SOURCES})
TARGET_LINK_LIBRARIES(wspp_util ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} ${CRYPTOPP_LIBRARIES} dl z pthread )

ADD_LIBRARY(wspp_web SHARED ${VIEWS_SOURCES} ${CONTROLLERS_SOURCES})
TARGET_LINK_LIBRARIES(wspp_web wspp_util ${Boost_LIBRARIES} )

ADD_LIBRARY(wspp_http_server SHARED ${SERVER_SOURCES})
TARGET_LINK_LIBRARIES(wspp_http_server wspp_util ${Boost_LIBRARIES} dl z pthread )



ADD_SUBDIRECTORY(apps)
ADD_SUBDIRECTORY(tools)

