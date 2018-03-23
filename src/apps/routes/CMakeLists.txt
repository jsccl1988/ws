FIND_PACKAGE(SPATIALITE REQUIRED)

INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
ADD_EXECUTABLE(routes_app
   app.cpp
    route_model.cpp
    route_model.hpp
    gpx_parser.cpp
    gpx_parser.hpp
    page_controller.cpp
    users_controller.cpp
    route_controller.cpp
    attachment_controller.cpp
    wpts_controller.cpp
    page_view.cpp
    auth.cpp
    login.cpp
)

TARGET_LINK_LIBRARIES(routes_app ${Boost_LIBRARIES} ${SQLITE3_LIBRARY} ${SPATIALITE_LIBRARY} wspp_http_server wspp_web wspp_util)



