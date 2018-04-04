INCLUDE_DIRECTORIES(
	${SQLITE3_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_DEFINITIONS( -std=c++11 )

ADD_EXECUTABLE(test_forms test_forms.cpp )
TARGET_LINK_LIBRARIES(test_forms wspp_util wspp_web  wspp_http_server ${Boost_LIBRARIES} dl z pthread)
