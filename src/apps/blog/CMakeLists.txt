ADD_EXECUTABLE(blog
    blog.cpp
    user_controller.cpp
    page_controller.cpp
)

TARGET_LINK_LIBRARIES(blog ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} wspp_http_server wspp_util)



