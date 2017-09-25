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

class RouteModel {
 public:

    RouteModel(Connection &con);

    Variant fetchMountain(const std::string &mountain) ;
    Variant fetchAll() ;

protected:
    void fetchMountains() ;

private:

    std::map<std::string, Mountain> mountains_ ;
    Connection &con_ ;


} ;






#endif
