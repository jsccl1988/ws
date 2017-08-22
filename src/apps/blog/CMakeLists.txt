ADD_EXECUTABLE(blog
    blog.cpp
    user_controller.cpp
)
                # TARGET        PREFIX    SOURCE
ADD_WS_TEMPLATES(blog blog_app
    templates/page.tpl
    templates/head.tpl
    templates/content.tpl
    templates/navbar.tpl
    )

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/blog_app)

TARGET_LINK_LIBRARIES(blog ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} wspp_http_server wspp_util)



