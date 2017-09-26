#ifndef __ROUTE_MODEL_HPP__
#define __ROUTE_MODEL_HPP__

#include <wspp/util/database.hpp>
#include <wspp/util/variant.hpp>

using wspp::util::sqlite::Connection ;
using wspp::util::Variant ;
using wspp::util::Dictionary ;

struct Mountain {
    Mountain(const std::string &name, double lat, double lon):
        lat_(lat), lon_(lon), name_(name) {}

    std::string name_ ;
    double lat_, lon_ ;
};

struct TrackPoint {
    double lat_, lon_, ele_ ;
};

struct TrackSegment {
    std::string name_ ;
    std::vector<TrackPoint> pts_ ;
};

struct Track {
    std::string name_ ;
    std::vector<TrackSegment> segments_ ;
} ;

struct Waypoint {
    double lat_, lon_, ele_ ;
    std::string name_, desc_ ;
};

class RouteModel {
 public:

    RouteModel(Connection &con);

    Variant fetchMountain(const std::string &mountain) const;
    Variant fetchAllByMountain() const;
    Variant fetch(const std::string &id) const;
    Variant fetchAttachments(const std::string &id) const;
    std::string getMountainName(const std::string &mountain_id) const;
    std::string getAttachmentTitle(const std::string &att_id) const;

    Variant fetchTrackGeoJSON(const std::string &route_id) const ;
    void fetchTracks(const std::string &route_id, std::vector<Track> &tracks, std::vector<Waypoint> &wpts) ;

    static std::string exportGpx(const std::vector<Track> &tracks, const std::vector<Waypoint> &wpts) ;
    static std::string exportKml(const std::vector<Track> &tracks, const std::vector<Waypoint> &wpts) ;

    std::string fetchTitle(const std::string &id) const ;

protected:
    void fetchMountains() ;

private:

    std::map<std::string, Mountain> mountains_ ;
    std::map<std::string, std::string> attachment_titles_ ;
    Connection &con_ ;


} ;






#endif
