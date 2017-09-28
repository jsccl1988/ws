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

Dictionary RouteModel::getMountainsDict() const {
    Dictionary res ;
    for( const auto &p: mountains_ ) {
        res.add(p.first, p.second.name_) ;
    }
    return res ;
}

bool RouteModel::updateInfo(const string &id, const string &title, const string &mountain_id) {
    sqlite::Statement stmt(con_, "UPDATE routes SET title = ?, mountain = ? WHERE id = ?");
    stmt(title, mountain_id, id) ;
    return true ;
}

bool RouteModel::getInfo(const string &id, string &title, string &mountain_id) {
    sqlite::Query stmt(con_, "SELECT title, mountain FROM routes WHERE id = ?");
    sqlite::QueryResult res = stmt(id) ;
    if ( res ) {
        res.into(title, mountain_id) ;
        return true ;
    }
    else return false ;
}


static string wkt_from_geom(const Track &track) {
    ostringstream geomstr ;

    geomstr << "MULTILINESTRING(" ;

    for ( uint k=0 ; k < track.segments_.size() ; k++ ) {

        if ( k > 0 ) geomstr << ',' ;
        const TrackSegment &seg = track.segments_[k] ;
        geomstr << '(' ;
        for( uint i=0 ; i<seg.pts_.size() ; i++ ) {
            const TrackPoint &pt = seg.pts_[i] ;
            if ( i > 0 ) geomstr << ',' ;
            geomstr << pt.lon_ << ' ' << pt.lat_ ;
        }
        geomstr << ')' ;
    }

    geomstr << ')' ;

    return geomstr.str() ;
}

static string wkt_from_geom(const Waypoint &pt) {
    ostringstream geomstr ;
    geomstr << "POINT(" << pt.lon_ << ' ' << pt.lat_ << ")" ;
    return geomstr.str() ;
}

bool RouteModel::importRoute(const string &title, const string &mountain_id, const RouteGeometry &geom)
{
    uint64_t route_id ;

    {
        sqlite::Statement stmt(con_, "INSERT INTO routes ( title, mountain ) VALUES (?, ?)", title, mountain_id) ;
        stmt.exec() ;
        route_id = con_.last_insert_rowid() ;
    }

    sqlite::Transaction trans(con_) ;

    sqlite::Statement stmt_tracks(con_, "INSERT INTO tracks ( geom, route ) VALUES (ST_GeomFromText(?,4326), ?)") ;

    for( const Track &track: geom.tracks_ ) {

        string geom = wkt_from_geom(track) ;

        stmt_tracks(geom, route_id) ;
        stmt_tracks.clear() ;
    }

    sqlite::Statement stmt_wpts(con_, "INSERT INTO wpts ( geom, route, name, desc, ele ) VALUES (ST_GeomFromText(?,4326), ?, ?, ?, ?)") ;

    for( const Waypoint &wpt: geom.wpts_ ) {

        string geom = wkt_from_geom(wpt) ;

        stmt_wpts(geom, route_id, wpt.name_, wpt.desc_, wpt.ele_) ;
        stmt_wpts.clear() ;
    }

    trans.commit() ;
}

Variant RouteModel::exportGeoJSON(const RouteGeometry &g) {

    Variant::Array track_features, wpt_features ;

    for( const Track &tr: g.tracks_ ) {

        Variant::Array track_coords ;

        for( const TrackSegment &seg: tr.segments_ ) {

            Variant::Array ls ;

            for( const TrackPoint &pt: seg.pts_ ) {
                ls.emplace_back(Variant::Array{pt.lon_, pt.lat_}) ;
            }

            track_coords.emplace_back(ls) ;
        }

        track_features.emplace_back(Variant::Object{{"type", "Feature"},
                                                    {"geometry", Variant::Object{{"type", "MultiLineString"}, {"coordinates", track_coords}}
                                                    }}) ;
    }

    for( const Waypoint &wpt: g.wpts_ ) {

        Variant::Array wpt_coords{ wpt.lon_, wpt.lat_ } ;

        wpt_features.emplace_back(Variant::Object{{"type", "Feature"},
                                                  {"name", wpt.name_ },
                                                  {"desc", wpt.desc_ },
                                                  {"geometry", Variant::Object{{"type", "Point"}, {"coordinates", wpt_coords}}}
                                  }) ;
    }


    Variant::Object tracks{{"type", "FeatureCollection"},
                           {"features", track_features}} ;

    Variant::Object wpts{{"type", "FeatureCollection"},
                         {"features", wpt_features}} ;

    Variant::Array box{g.box_.min_lon_, g.box_.min_lat_, g.box_.max_lon_, g.box_.max_lat_} ;

    return Variant::Object{{"box", box}, {"tracks", tracks }, {"wpts", wpts} } ;
}


static GeomBox box_from_extent(const sqlite::Blob &blob) {

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

    return {min_lon, min_lat, max_lon, max_lat} ;
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

    uint count = 1 ;
    while ( line ) {
        TrackSegment segment ;
        segment.name_ = "segment-" + to_string(count++) ;
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

void RouteModel::fetchGeometry(const string &route_id, RouteGeometry &g)
{
    string name = g.name_ = fetchTitle(route_id) ;
    {
        sqlite::Query stmt(con_, "SELECT id, geom FROM tracks WHERE route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;

        while ( res ) {
            Track tr ;

            sqlite::Blob blob = res.get<sqlite::Blob>("geom") ;
            gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb ((const unsigned char *)blob.data(), blob.size());
            parse_multi_linestring(geom, tr) ;
            gaiaFreeGeomColl(geom);

            g.tracks_.emplace_back(tr) ;
            res.next() ;
        }

        if ( g.tracks_.size() == 1 ) g.tracks_[0].name_ = name ;
        else  {
            uint k = 1 ;
            for( Track &tr: g.tracks_ )
                tr.name_ = name + "_" + to_string(k++) ;
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

            g.wpts_.emplace_back(pt) ;
            res.next() ;
        }
    }

    {

        sqlite::Query stmt(con_, "SELECT Extent(geom) as box FROM tracks where route=?") ;
        sqlite::QueryResult res = stmt(route_id) ;
        sqlite::Blob blob = res.get<sqlite::Blob>(0) ;

        if ( blob.data() != nullptr ) {
            g.box_ = box_from_extent(blob) ;
        }
    }

}

bool RouteModel::remove(const string &id) {
    sqlite::Statement(con_, "DELETE FROM routes WHERE id = ?", id).exec() ;
    sqlite::Statement(con_, "DELETE FROM tracks WHERE id = ?", id).exec() ;
    sqlite::Statement(con_, "DELETE FROM wpts WHERE id = ?", id).exec() ;
    return true ;
}

static string xml_date() {
    char time_buf[21];
    time_t now;
    std::time(&now);
    std::strftime(time_buf, 21, "%Y-%m-%dT%H:%S:%MZ", std::gmtime(&now));
    return time_buf ;
}

string RouteModel::exportGpx(const RouteGeometry &g) {
    ostringstream strm ;

    XmlWriter writer(strm) ;

    writer.startDocument();
    writer.startElement("gpx") ;
    writer.attr("version", "1.0")
            .attr("creator", "http://vision.iti.gr/routes/")
            .attr("xmlns", "http://www.topografix.com/GPX/1/0")
            .attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
            .attr("xsi:schemaLocation", "http://www.topografix.com/GPX/1/0/gpx.xsd") ;

    writer.startElement("time") ;
    writer.text(xml_date()) ;
    writer.endElement() ;

    writer.startElement("bounds")
            .attr("minlat", to_string(g.box_.min_lat_))
            .attr("minlon", to_string(g.box_.min_lon_))
            .attr("maxlat", to_string(g.box_.max_lat_))
            .attr("maxlon", to_string(g.box_.max_lon_))
            .endElement() ;

    for ( const Waypoint &wpt: g.wpts_ ) {
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

    for( const Track &trk: g.tracks_ ) {
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

string RouteModel::exportKml(const RouteGeometry &g) {

    ostringstream strm ;
    XmlWriter writer(strm) ;

    writer.startDocument();
    writer.startElement("kml") ;

    writer.attr("xmlns", "http://www.opengis.net/kml/2.2")
            .attr("xmlns:gx", "http://www.google.com/kml/ext/2.2")
            .attr("xmlns:kml", "http://www.opengis.net/kml/2.2")
            .attr("xmlns:atom", "http://www.w3.org/2005/Atom") ;

    writer.startElement("Document") ;

    writer.startElement("name") ;
    writer.text(g.name_);
    writer.endElement() ;

    // style for waypoints

    writer.startElement("Style").attr("id", "waypoint") ;
    writer.startElement("IconStyle") ;
    writer.startElement("Icon") ;
    writer.startElement("href") ;
    writer.text("http://maps.google.com/mapfiles/kml/paddle/red-stars.png") ;
    writer.endElement() ;
    writer.endElement() ;
    writer.endElement() ;
    writer.endElement() ;

    // styles for tracks

    writer.startElement("Style").attr("id", "track") ;
    writer.startElement("LineStyle") ;
    writer.startElement("color") ;
    writer.text("ffff0000") ;
    writer.endElement() ;
    writer.startElement("width") ;
    writer.text("2") ;
    writer.endElement() ;
    writer.endElement() ;
    writer.endElement() ;

    // waypoints

    writer.startElement("Folder") ;

    writer.startElement("name") ;
    writer.text("Waypoints") ;
    writer.endElement() ;

    writer.startElement("Region") ;
    writer.startElement("LatLonAltBox") ;
    writer.startElement("east") ;
    writer.text(to_string(g.box_.max_lon_)) ;
    writer.endElement() ;
    writer.startElement("west") ;
    writer.text(to_string(g.box_.min_lon_)) ;
    writer.endElement() ;
    writer.startElement("north") ;
    writer.text(to_string(g.box_.max_lat_)) ;
    writer.endElement() ;
    writer.startElement("south") ;
    writer.text(to_string(g.box_.min_lat_)) ;
    writer.endElement() ;
    writer.endElement() ;

    writer.startElement("Lod") ;
    writer.startElement("minLodPixels") ;
    writer.text("256") ;
    writer.endElement() ;
    writer.endElement() ;
    writer.endElement() ;


    for ( const Waypoint &pt: g.wpts_ ) {
        ostringstream coordinates ;
        coordinates << pt.lon_ << ',' << pt.lat_ ;
        if ( pt.ele_ > 0 ) coordinates << ',' << pt.ele_ ;

        writer.startElement("Placemark") ;
        writer.startElement("name") ;
        writer.text(boost::trim_copy(pt.name_)) ;
        writer.endElement() ;

        if ( !pt.desc_.empty() ) {
            writer.startElement("description") ;
            writer.text(boost::trim_copy(pt.desc_)) ;
            writer.endElement() ;
        }

        writer.startElement("styleUrl") ;
        writer.text("#waypoint") ;
        writer.endElement() ;

        writer.startElement("Point") ;
        writer.startElement("coordinates") ;
        writer.text(coordinates.str()) ;
        writer.endElement() ;
        writer.endElement() ;
        writer.endElement() ;
    }

    writer.endElement() ; // waypoints folder

    // Tracks

    writer.startElement("Folder") ;

    writer.startElement("name") ;
    writer.text("Tracks") ;
    writer.endElement() ;

    uint count = 1 ;


    for( const Track &trk: g.tracks_ ) {

        writer.startElement("Folder") ;

        writer.startElement("name") ;
        writer.text(boost::trim_copy(trk.name_)) ;
        writer.endElement() ;

        for( const TrackSegment &seg: trk.segments_ ) {

            writer.startElement("Placemark") ;

            writer.startElement("name") ;
            writer.text(boost::trim_copy(seg.name_)) ;
            writer.endElement() ;

            writer.startElement("styleUrl") ;
            writer.text("#track") ;
            writer.endElement() ;

            writer.startElement("LineString") ;
            writer.startElement("coordinates") ;

            ostringstream coordinates ;

            for( const TrackPoint &pt: seg.pts_ ) {
                coordinates << pt.lon_ << ',' << pt.lat_ << ' ' ;
            }

            writer.text(coordinates.str()) ;
            writer.endElement() ;
            writer.endElement() ;
            writer.endElement() ;
        }

        writer.endElement() ; // trkseg
    }

    writer.endElement() ; // tracks folder

    writer.endElement() ; // Document
    writer.endElement() ; // kml

    writer.endDocument();

    return strm.str() ;
}

string RouteModel::fetchTitle(const string &id) const {
    return sqlite::Query(con_, "SELECT title FROM routes WHERE id=?", id).exec().get<string>("title") ;
}

Variant RouteModel::query(double x, double y) const {
    sqlite::Query q(con_, "SELECT r.id, r.title FROM routes AS r JOIN tracks as t ON t.route = r.id WHERE ST_Intersects(ST_Transform(SetSRID(t.geom, 4326), 3857), ST_Buffer(MakePoint(?, ?, 3857), 20)) LIMIT 1") ;
    sqlite::QueryResult row = q(x, y) ;
    if ( row )
        return Variant::Object{{"id", row.get<uint>("id")}, {"title", row.get<string>("title")}} ;
    else
        return Variant();
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
    return Variant() ;
}

Variant RouteModel::fetchWaypoints(const string &route_id) const {

    sqlite::Query stmt(con_, "SELECT id, ele, name, desc, ST_X(geom) as lon, ST_Y(geom) as lat FROM wpts where route=?") ;
    sqlite::QueryResult res = stmt(route_id) ;

    Variant::Array wpts ;

    int id ;
    string name, desc ;
    double ele, lon, lat ;

    while ( res ) {
        res.into(id, ele, name, desc, lon, lat) ;
        Variant::Object results{{"id", id}, {"name", name}, {"desc", desc}, {"lat", lat}, {"lon", lon}, {"ele", ele}} ;
        wpts.emplace_back(results) ;
        res.next() ;
    }

    return wpts ;
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
