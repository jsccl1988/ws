#ifndef __ROUTE_MODEL_HPP__
#define __ROUTE_MODEL_HPP__

#include <wspp/util/database.hpp>
#include <wspp/util/variant.hpp>

#include "route_geometry.hpp"

using wspp::util::sqlite::Connection ;
using wspp::util::Variant ;
using wspp::util::Dictionary ;

struct Mountain {
    Mountain(const std::string &name, double lat, double lon):
        lat_(lat), lon_(lon), name_(name) {}

    std::string name_ ;
    double lat_, lon_ ;
};


class RouteModel {
 public:

    RouteModel(Connection &con);

    Variant fetchMountain(const std::string &mountain) const;
    Variant fetchAllByMountain() const;
    Variant fetch(const std::string &id) const;
    Variant fetchWaypoints(const std::string &id) const;
    Variant fetchAttachments(const std::string &id) const;
    std::string getMountainName(const std::string &mountain_id) const;
    std::string getAttachmentTitle(const std::string &att_id) const;
    Dictionary getMountainsDict() const ;
    Dictionary getAttachmentsDict() const { return attachment_titles_ ; }

    bool updateInfo(const std::string &id, const std::string &title, const std::string &mountain_id) ;
    bool updateAttachment(const std::string &id, const std::string &name, const std::string &type_id) ;
    bool updateWaypoint(const std::string &id, const std::string &name, const std::string &desc) ;
    bool getInfo(const std::string &id, std::string &title, std::string &mountain_id) ;
    bool getAttachment(const std::string &id, std::string &name, std::string &type_id) ;
    bool getWaypoint(const std::string &id, std::string &name, std::string &desc) ;

    bool importRoute(const std::string &title, const std::string &role_id, const RouteGeometry &geom) ;
    bool createAttachment(const std::string &route_id, const std::string &name, const std::string &type_id, const std::string &data, const std::string &upload_folder) ;

    void fetchGeometry(const std::string &route_id, RouteGeometry &geom) ;

    bool remove(const std::string &id) ;
    bool removeAttachment(const std::string &id) ;
    bool removeWaypoint(const std::string &id) ;

    static Variant exportGeoJSON(const RouteGeometry &geom) ;
    static std::string exportGpx(const RouteGeometry &geom) ;
    static std::string exportKml(const RouteGeometry &geom) ;

    std::string fetchTitle(const std::string &id) const ;
    Variant query(double x, double y) const ;


protected:
    void fetchMountains() ;

private:

    std::map<std::string, Mountain> mountains_ ;
    std::map<std::string, std::string> attachment_titles_ ;
    Connection &con_ ;


} ;






#endif
