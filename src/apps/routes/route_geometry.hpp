#ifndef __ROUTE_GEOMETRY_HPP__
#define __ROUTE_GEOMETRY_HPP__

struct TrackPoint {
    double lat_, lon_, ele_;
};

struct TrackSegment {
    std::string name_;
    std::vector<TrackPoint> pts_;
};

struct Track {
    std::string name_;
    std::vector<TrackSegment> segments_;
};

struct Waypoint {
    double lat_, lon_, ele_;
    std::string name_, desc_;
};

struct GeomBox {
    GeomBox() {
        min_lat_ = min_lon_ = std::numeric_limits<double>::max();
        max_lat_ = max_lon_ = -std::numeric_limits<double>::max();
    }
    GeomBox(double min_lon, double min_lat, double max_lon, double max_lat):
        min_lon_(min_lon), min_lat_(min_lat), max_lon_(max_lon), max_lat_(max_lat) {}

    double min_lat_, min_lon_, max_lat_, max_lon_;
};

struct RouteGeometry {
    std::string name_;
    GeomBox box_;
    std::vector<Track> tracks_;
    std::vector<Waypoint> wpts_;
};
#endif
