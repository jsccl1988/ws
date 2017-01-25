SET ( SERVER_SOURCES

	${SRC_ROOT}/util/dictionary.cpp
	${SRC_ROOT}/util/dictionary.hpp

	${SRC_ROOT}/util/logger.cpp
	${SRC_ROOT}/util/logger.hpp


	${SRC_ROOT}/server/connection.cpp
	${SRC_ROOT}/server/connection.hpp
	${SRC_ROOT}/server/connection_manager.hpp
	${SRC_ROOT}/server/connection_manager.cpp
	${SRC_ROOT}/server/io_service_pool.cpp
	${SRC_ROOT}/server/io_service_pool.hpp
	${SRC_ROOT}/server/reply.cpp
	${SRC_ROOT}/server/reply.hpp
	${SRC_ROOT}/server/request_handler.hpp
	${SRC_ROOT}/server/request.hpp
	${SRC_ROOT}/server/request_parser.cpp
	${SRC_ROOT}/server/request_parser.hpp
	${SRC_ROOT}/server/server.cpp
	${SRC_ROOT}/server/server.hpp
	${SRC_ROOT}/server/http_parser.c
	${SRC_ROOT}/server/http_parser.h
	${SRC_ROOT}/server/request_handler_factory.hpp

	
)

ADD_LIBRARY(ws_http_server STATIC ${SERVER_SOURCES})
TARGET_LINK_LIBRARIES(ws_http_server ${Boost_LIBRARIES} dl z pthread )

ADD_EXECUTABLE(wsrv ${SRC_ROOT}/server/tile_server.cpp)
TARGET_LINK_LIBRARIES(wsrv ws_http_server)

