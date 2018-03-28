SET( UTIL_SOURCES

    ${SRC_ROOT}/util/dictionary.cpp
    ${SRC_ROOT}/util/logger.cpp

    ${SRC_ROOT}/util/crypto.cpp
    ${SRC_ROOT}/util/zfstream.cpp
    ${SRC_ROOT}/util/json.cpp
    ${SRC_ROOT}/util/filesystem.cpp
    ${SRC_ROOT}/util/xml_writer.cpp
    ${SRC_ROOT}/util/xml_sax_parser.cpp
    ${SRC_ROOT}/util/i18n.cpp

    ${SRC_ROOT}/database/connection.cpp
    ${SRC_ROOT}/database/connection_handle.cpp
    ${SRC_ROOT}/database/driver_factory.cpp
    ${SRC_ROOT}/database/exception.cpp
    ${SRC_ROOT}/database/statement.cpp
    ${SRC_ROOT}/database/transaction.cpp
    ${SRC_ROOT}/database/query.cpp
    ${SRC_ROOT}/database/query_result.cpp
    ${SRC_ROOT}/database/statement_handle.cpp

    ${SRC_ROOT}/database/drivers/sqlite/driver.cpp
    ${SRC_ROOT}/database/drivers/sqlite/connection.cpp
    ${SRC_ROOT}/database/drivers/sqlite/exceptions.cpp
    ${SRC_ROOT}/database/drivers/sqlite/statement.cpp
    ${SRC_ROOT}/database/drivers/sqlite/query_result.cpp



    ${INCLUDE_ROOT}/util/dictionary.hpp
    ${INCLUDE_ROOT}/util/logger.hpp
    ${INCLUDE_ROOT}/util/crypto.hpp
    ${INCLUDE_ROOT}/util/zfstream.hpp
    ${INCLUDE_ROOT}/util/filesystem.hpp
    ${INCLUDE_ROOT}/util/xml_writer.hpp
    ${INCLUDE_ROOT}/util/xml_sax_parser.hpp
    ${INCLUDE_ROOT}/util/i18n.hpp
)

IF ( PostgreSQL_FOUND )
    ADD_DEFINITIONS("-DHAS_PGSQL_DRIVER")
    LIST(APPEND UTIL_SOURCES
        ${SRC_ROOT}/database/drivers/pgsql/driver.cpp
        ${SRC_ROOT}/database/drivers/pgsql/connection.cpp
        ${SRC_ROOT}/database/drivers/pgsql/exceptions.cpp
        ${SRC_ROOT}/database/drivers/pgsql/statement.cpp
        ${SRC_ROOT}/database/drivers/pgsql/query_result.cpp
        ${SRC_ROOT}/database/drivers/pgsql/parameters.cpp
)
ENDIF ( PostgreSQL_FOUND )

SET ( WEB_SOURCES
    ${SRC_ROOT}/views/renderer.cpp
    ${SRC_ROOT}/views/template_parser.cpp
    ${SRC_ROOT}/views/template_parser.hpp
    ${SRC_ROOT}/views/forms.cpp
    ${SRC_ROOT}/views/table.cpp
    ${SRC_ROOT}/views/menu.cpp
    ${SRC_ROOT}/views/validators.cpp

 #   ${SRC_ROOT}/controllers/login.cpp

 #   ${SRC_ROOT}/models/auth.cpp

    ${INCLUDE_ROOT}/views/renderer.hpp

    ${INCLUDE_ROOT}/views/forms.hpp
    ${INCLUDE_ROOT}/views/table.hpp
    ${INCLUDE_ROOT}/views/menu.hpp
    ${INCLUDE_ROOT}/views/validators.hpp

#    ${INCLUDE_ROOT}/controllers/login.hpp

#    ${INCLUDE_ROOT}/models/auth.hpp
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
    ${INCLUDE_ROOT}/server/route.hpp
    ${INCLUDE_ROOT}/server/router.hpp
    ${INCLUDE_ROOT}/server/filter.hpp
    ${INCLUDE_ROOT}/server/exceptions.hpp
    ${INCLUDE_ROOT}/server/filters/request_logger.hpp
    ${INCLUDE_ROOT}/server/filters/static_file_handler.hpp
    ${INCLUDE_ROOT}/server/filters/gzip_filter.hpp

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
    ${SRC_ROOT}/server/route.cpp
    ${SRC_ROOT}/server/filter_chain.cpp
    ${SRC_ROOT}/server/exceptions.cpp

    ${SRC_ROOT}/server/filters/request_logger.cpp
    ${SRC_ROOT}/server/filters/static_file_handler.cpp
    ${SRC_ROOT}/server/filters/gzip_filter.cpp
)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/template_parser/)

FLEX_TARGET(TEMPLATE_SCANNER ${SRC_ROOT}/template/template.l  ${CMAKE_CURRENT_BINARY_DIR}/template_parser/flex_scanner.cpp)
BISON_TARGET(TEMPLATE_PARSER ${SRC_ROOT}/template/template.y  ${CMAKE_CURRENT_BINARY_DIR}/template_parser/bison_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(TEMPLATE_SCANNER TEMPLATE_PARSER COMPILE_FLAGS "-Wconflicts-sr")

LIST(APPEND WEB_SOURCES ${FLEX_TEMPLATE_SCANNER_OUTPUTS} ${BISON_TEMPLATE_PARSER_OUTPUTS}
#    ${SRC_ROOT}/template/template_eval_context.cpp
#    ${SRC_ROOT}/template/template_eval_context.hpp

    ${SRC_ROOT}/template/parser.hpp
    ${SRC_ROOT}/template/parser.cpp

    ${SRC_ROOT}/template/scanner.hpp

    ${SRC_ROOT}/template/template_ast.hpp
    ${SRC_ROOT}/template/template_ast.cpp

    ${SRC_ROOT}/template/template_loader.hpp
    ${SRC_ROOT}/template/template_loader.cpp

    ${SRC_ROOT}/template/template_renderer.hpp
    ${SRC_ROOT}/template/template_renderer.cpp
)

INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/template/
)

ADD_LIBRARY(wspp_util SHARED ${UTIL_SOURCES})
TARGET_LINK_LIBRARIES(wspp_util ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} ${CRYPTOPP_LIBRARIES} ${PostgreSQL_LIBRARIES} dl z pthread )

ADD_LIBRARY(wspp_web SHARED ${WEB_SOURCES})
TARGET_LINK_LIBRARIES(wspp_web wspp_util ${Boost_LIBRARIES} )

ADD_LIBRARY(wspp_http_server SHARED ${SERVER_SOURCES})
TARGET_LINK_LIBRARIES(wspp_http_server wspp_util ${Boost_LIBRARIES} dl z pthread )

ADD_EXECUTABLE(test_parser ${SRC_ROOT}/template/test_parser.cpp)
TARGET_LINK_LIBRARIES(test_parser wspp_util wspp_web wspp_http_server ${Boost_LIBRARIES} dl z pthread )

ADD_SUBDIRECTORY(apps)
ADD_SUBDIRECTORY(tools)

