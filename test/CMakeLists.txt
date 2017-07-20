INCLUDE_DIRECTORIES(
	${SQLITE3_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_DEFINITIONS( -std=c++11 )

#ADD_EXECUTABLE(test_variant test_variant.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/util/variant.hpp)
