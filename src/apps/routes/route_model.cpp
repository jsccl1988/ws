#include "route_model.hpp"

#include <wspp/util/xml_writer.hpp>

#include <boost/algorithm/string.hpp>

#include <spatialite.h>

#include <ctime>

using namespace std ;
using namespace wspp::util ;

RouteModel::RouteModel(sqlite::Connection &con): con_(con) {
    fetchMountains() ;
    attachment_titles_ = {{"sketch", "Σκαρίφημα"}, {"description", "Περιγραφή"}} ;
}

Variant RouteModel::fetchMountain(const string &mountain) const
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

string RouteModel::getMountainName(const string &mountain_id) const {
    auto it = mountains_.find(mountain_id) ;
    assert(it != mountains_.end()) ;
    return it->second.name_ ;
}

string RouteModel::getAttachmentTitle(const string &att_id) const {
    auto it = attachment_titles_.find(att_id) ;
    assert(it != attachment_titles_.end()) ;
    return it->second ;
}

static Variant::Array box_from_extent(const sqlite::Blob &blob) {

    double min_lat, min_lon, max_lat, max_lon ;

    min_lat = min_lon = std::numeric_limits<double>::max() ;
    max_lat = max_lon = -std::numeric_limits<double>::max() ;

    gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb ((const unsigned char *)blob.data(), blob.size());

    gaiaPolygonPtr poly = geom->FirstPolygon ;
    gaiaRingPtr ring = poly->Exterior ;

    double *c = ring->Coords ;

    for( uint i=0 ; i<ring->Points ; i++ ) {
        double lon = *c++, lat = *c++ ;

        min_lat = std::min<float>(min_lat, lat) ;
        min_lon = std::min<float>(min_lon, lon) ;
        max_lat = std::max<float>(max_lat, lat) ;
        max_lon = std::max<float>(max_lon, lon) ;
    }

    return Variant::Array({min_lon, min_lat, max_lon, max_lat}) ;
}

Variant RouteModel::fetchTrackGeoJSON(const string &route_id) const {


    Variant::Array track_features, wpt_features, box ;

    {
        sqlite::Query stmt(con_, "SELECT id, AsGeoJSON(geom) as geom FROM tracks WHERE route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;

        while ( res ) {
            track_features.emplace_back(Variant::Object{{"type", "Feature"},
                                                  {"id", res.get<int>("id")},
                                                  {"geometry", Variant::fromJSONString(res.get<string>("geom"))}}) ;
            res.next() ;
        }
    }

    {
        sqlite::Query stmt(con_, "SELECT id, name, desc, AsGeoJSON(geom) as geom FROM wpts WHERE route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;

        while ( res ) {
            wpt_features.emplace_back(Variant::Object{{"type", "Feature"},
                                                  {"id", res.get<int>("id")},
                                                  {"name", res.get<string>("name") },
                                                  {"desc", res.get<string>("desc") },
                                                  {"geometry", Variant::fromJSONString(res.get<string>("geom"))}}) ;
            res.next() ;
        }
    }



    Variant::Object tracks{{"type", "FeatureCollection"},
                           {"features", track_features}} ;

    Variant::Object wpts{{"type", "FeatureCollection"},
                           {"features", wpt_features}} ;


    {

        sqlite::Query stmt(con_, "SELECT Extent(geom) as box FROM tracks where route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;
        sqlite::Blob blob = res.get<sqlite::Blob>(0) ;

        if ( blob.data() != nullptr )
            box = box_from_extent(blob) ;
    }

    return Variant::Object{{"box", box}, {"tracks", tracks }, {"wpts", wpts} } ;
}

static void parse_linestring(gaiaLinestringPtr line, TrackSegment &seg) {
    size_t n_points = line->Points ;
    double *pts = line->Coords ;

    for( size_t i=0 ; i<n_points ; i++ ) {
        TrackPoint pt ;
        pt.lon_ = *pts++;
        pt.lat_ = *pts++ ;
        seg.pts_.emplace_back(pt) ;
     }
}

static void parse_multi_linestring(gaiaGeomCollPtr geom, Track &track) {

    gaiaLinestringPtr line = geom->FirstLinestring ;

    while ( line ) {
        TrackSegment segment ;
        parse_linestring(line, segment) ;
        track.segments_.emplace_back(segment) ;
        line = line->Next ;
    }

}

static void parse_point(gaiaGeomCollPtr geom, Waypoint &wpt) {

    gaiaPointPtr pt = geom->FirstPoint ;
    if ( pt ) {
        wpt.lon_ = pt->X ;
        wpt.lat_ = pt->Y ;
    }
}

void RouteModel::fetchTracks(const string &route_id, std::vector<Track> &tracks, std::vector<Waypoint> &wpts)
{
    {
        sqlite::Query stmt(con_, "SELECT id, geom FROM tracks WHERE route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;

        while ( res ) {
            Track tr ;

            sqlite::Blob blob = res.get<sqlite::Blob>("geom") ;
            gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb ((const unsigned char *)blob.data(), blob.size());
            parse_multi_linestring(geom, tr) ;
            gaiaFreeGeomColl(geom);

            tracks.emplace_back(tr) ;
            res.next() ;
        }
    }

    {
        sqlite::Query stmt(con_, "SELECT id, name, ele, desc, geom FROM wpts WHERE route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;

        while ( res ) {
            Waypoint pt ;

            sqlite::Blob blob = res.get<sqlite::Blob>("geom") ;
            gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb ((const unsigned char *)blob.data(), blob.size());
            parse_point(geom, pt) ;
            pt.ele_ = res.get<double>("ele") ;
            pt.name_ = res.get<string>("name") ;
            pt.desc_ = res.get<string>("desc") ;
            gaiaFreeGeomColl(geom);

            wpts.emplace_back(pt) ;
            res.next() ;
        }
    }

}

static string xml_date() {
    char time_buf[21];
    time_t now;
    std::time(&now);
    std::strftime(time_buf, 21, "%Y-%m-%dT%H:%S:%MZ", std::gmtime(&now));
    return time_buf ;
}

string RouteModel::exportGpx(const std::vector<Track> &tracks, const std::vector<Waypoint> &wpts) {
    ostringstream strm ;

    XmlWriter writer(strm) ;

    writer.startDocument();
    writer.startElement("gpx") ;
    writer.attr("version", "1.0")
          .attr("creator", "http://vision.iti.gr/routes/")
          .attr("xmlns", "http://www.topografix.com/GPX/1/0")
          .attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
          .attr("xsi:schemaLocation", "http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd") ;

    writer.startElement("time") ;
    writer.text(xml_date()) ;
    writer.endElement() ;
    /*
    writer.startElement('bounds') ;
            writer.writeAttribute('minlat', $data['box'][1]) ;
            writer.writeAttribute('minlon', $data['box'][0]) ;
            writer.writeAttribute('maxlat', $data['box'][3]) ;
            writer.writeAttribute('maxlon', $data['box'][2]) ;
            writer.endElement() ;
      */

    for ( const Waypoint &wpt: wpts ) {
        writer.startElement("wpt")
              .attr("lat", to_string(wpt.lat_) )
              .attr("lon", to_string(wpt.lon_) )  ;

        writer.startElement("ele")
              .text(to_string(wpt.ele_))
              .endElement() ;

        writer.startElement("name")
              .text(boost::trim_copy(wpt.name_))
              .endElement() ;

        writer.startElement("desc")
              .text(boost::trim_copy(wpt.desc_))
              .endElement() ;

        writer.endElement() ; //wpt
    }

    for( const Track &trk: tracks ) {
        writer.startElement("trk") ;

        if ( !trk.name_.empty() )
            writer.startElement("name").text(boost::trim_copy(trk.name_)).endElement() ;

        for( const TrackSegment &seg: trk.segments_ ) {
            writer.startElement("trkseg") ;

            for( const TrackPoint &pt: seg.pts_ ) {
                writer.startElement("trkpt")
                      .attr("lat", to_string(pt.lat_) )
                      .attr("lon", to_string(pt.lon_) )
                      .endElement()  ;
            }

            writer.endElement() ; // trkseg
        }

        writer.endElement() ; // trk
    }

    writer.endElement() ; // gpx
    writer.endDocument();

    return strm.str() ;
}

string RouteModel::exportKml(const std::vector<Track> &tracks, const std::vector<Waypoint> &wpts)
{

}

string RouteModel::fetchTitle(const string &id) const {
    return sqlite::Query(con_, "SELECT title FROM routes WHERE id=?", id).exec().get<string>("title") ;
}

Variant RouteModel::fetchAllByMountain() const
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
            results.emplace_back(Variant::Object{{"name", getMountainName(cmountain)}, {"routes", entry}}) ;
            cmountain = mountain ;
            entry.clear() ;
        }

        entry.emplace_back(Variant::Object{{"id", id},  {"title", title}}) ;

        res.next() ;
    }
    return results ;

}

Variant RouteModel::fetch(const string &id) const {
    sqlite::Query stmt(con_, "SELECT title, description, mountain FROM routes WHERE id = ? LIMIT 1") ;
    sqlite::QueryResult res = stmt(id) ;

    string title, description, mountain ;
    if ( res ) {
        res.into(title, description, mountain) ;
        Variant::Object results{{"id", id}, {"title", title}, {"description", description}, {"mountain", getMountainName(mountain)}} ;
        return results ;
    }
    return nullptr ;
}

// CREATE table attachments ( id INTEGER PRIMARY KEY AUTOINCREMENT, route INTEGER NOT NULL, type TEXT NOT NULL, name TEXT, url TEXT, FOREIGN KEY (route) REFERENCES routes(id)) ;
// CREATE TABLE tracks ( id INTEGER PRIMARY KEY AUTOINCREMENT, route INTEGER NOT NULL, FOREIGN KEY (route) REFERENCES routes(id))
// SELECT AddGeometryColumn('tracks', 'geom', 4326, 'MULTILINESTRING', 'XY');
// CREATE table wpts ( id INTEGER PRIMARY KEY AUTOINCREMENT, route INTEGER NOT NULL, ele DOUBLE, name TEXT, desc TEXT, FOREIGN KEY (route) REFERENCES routes(id))
// SELECT AddGeometryColumn('wpts', 'geom', 4326, 'POINT', 'XY');
// SELECT CreateSpatialIndex('wpts', 'geom');

Variant RouteModel::fetchAttachments(const string &route_id) const  {
    sqlite::Query stmt(con_, "SELECT id, type, name, url FROM attachments WHERE route = ?") ;
    sqlite::QueryResult res = stmt(route_id) ;

    string id, type, name, url  ;

    Variant::Array results ;
    while ( res ) {
        res.into(id, type, name, url) ;
        results.emplace_back(Variant::Object{{"id", id}, {"title", getAttachmentTitle(type)}, {"name", name}, {"url", url}}) ;
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
