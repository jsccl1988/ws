#include "route_model.hpp"

using namespace std ;
using namespace wspp::util ;

RouteModel::RouteModel(sqlite::Connection &con): con_(con) {
    fetchMountains() ;
}

Variant RouteModel::fetchMountain(const string &mountain)
{
    Variant::Array results ;

    sqlite::Query stmt(con_, "SELECT id, title FROM routes WHERE mountain = ?") ;

    sqlite::QueryResult res = stmt(mountain) ;

    string title ;
    int id ;
    while ( res ) {
        res.into(id, title) ;
        results.emplace_back(Variant::Object{ {"id", id},  {"title", title} }) ;
        res.next() ;
    }
    return results ;
}

Variant RouteModel::fetchAll()
{
    Variant::Array results ;

    sqlite::Query stmt(con_, "SELECT id, title, mountain FROM routes ORDER BY mountain") ;

    sqlite::QueryResult res = stmt.exec() ;

    string title, mountain, cmountain ;
    int id ;

    Variant::Array entry ;
    while ( res ) {
        res.into(id, title, mountain) ;
        if ( cmountain.empty() ) cmountain = mountain ;
        if ( mountain != cmountain ) {
            auto it = mountains_.find(cmountain) ;
            assert(it != mountains_.end()) ;
            results.emplace_back(Variant::Object{{"name", it->second.name_}, {"routes", entry}}) ;
            cmountain = mountain ;
            entry.clear() ;
        }

        entry.emplace_back(Variant::Object{{"id", id},  {"title", title}}) ;

        res.next() ;
    }
    return results ;

}

void RouteModel::fetchMountains()
{
    sqlite::Query stmt(con_, "SELECT * FROM mountains") ;

    for( auto &m: stmt() ) {
        mountains_.emplace(m["id"].as<string>(), Mountain(m["name"].as<string>(), m["lat"].as<double>(), m["lon"].as<double>())) ;
    }
}
