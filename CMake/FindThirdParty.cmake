FIND_PACKAGE(Sqlite3 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

# Boost

set(BOOST_COMPONENTS regex filesystem system serialization coroutine context program_options)

FIND_PACKAGE(Boost 1.49 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

