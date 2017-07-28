SET( UTIL_SOURCES

    ${SRC_ROOT}/util/dictionary.cpp
    ${INCLUDE_ROOT}/util/dictionary.hpp

    ${SRC_ROOT}/util/logger.cpp
    ${INCLUDE_ROOT}/util/logger.hpp

    ${SRC_ROOT}/util/database.cpp
    ${INCLUDE_ROOT}/util/database.hpp

    ${SRC_ROOT}/util/crypto.cpp
    ${INCLUDE_ROOT}/util/crypto.hpp
)

ADD_LIBRARY(wspp_util SHARED ${UTIL_SOURCES})
TARGET_LINK_LIBRARIES(wspp_util ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} dl z pthread )


SET( AUTH_SOURCES

    ${SRC_ROOT}/auth/password_authenticator.cpp
    ${INCLUDE_ROOT}/auth/password_authenticator.hpp
)

ADD_LIBRARY(wspp_auth SHARED ${AUTH_SOURCES})
TARGET_LINK_LIBRARIES(wspp_auth ${Boost_LIBRARIES}  dl z pthread )

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

ADD_LIBRARY(wspp_http_server SHARED ${SERVER_SOURCES})
TARGET_LINK_LIBRARIES(wspp_http_server wspp_util ${Boost_LIBRARIES} ${CRYPTOPP_LIBRARIES} dl z pthread )



