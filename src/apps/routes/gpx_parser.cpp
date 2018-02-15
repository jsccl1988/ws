#include "gpx_parser.hpp"
#include "route_model.hpp"

#include <boost/algorithm/string.hpp>

using namespace std ;

GpxParser::GpxParser(const string &src, RouteGeometry &geom): XMLSAXParser(src), geom_(geom) {
}

void GpxParser::startElement(const std::string &qname, const XMLSAXParser::AttributeList &attrs) {
    if ( qname == "gpx" ) {
        is_gpx_document_ = true ;
        ctx_.push(DocumentContext) ;
    } else if ( qname == "wpt" ) {
        current_wpt_.lat_ = attrs.value<double>("lat", 0) ;
        current_wpt_.lon_ = attrs.value<double>("lon", 0) ;

        current_wpt_.desc_.clear() ;
        current_wpt_.name_.clear() ;

        geom_.box_.min_lon_ = std::min(geom_.box_.min_lon_, current_wpt_.lon_) ;
        geom_.box_.min_lat_ = std::min(geom_.box_.min_lat_, current_wpt_.lat_) ;
        geom_.box_.max_lon_ = std::max(geom_.box_.max_lon_, current_wpt_.lon_) ;
        geom_.box_.max_lon_ = std::max(geom_.box_.min_lon_, current_wpt_.lat_) ;

        ctx_.push(WayPointContext) ;
    } else if ( qname == "trkpt" ) {
        current_tpt_.lat_ = attrs.value<double>("lat", 0) ;
        current_tpt_.lon_ = attrs.value<double>("lon", 0) ;

        geom_.box_.min_lon_ = std::min(geom_.box_.min_lon_, current_tpt_.lon_) ;
        geom_.box_.min_lat_ = std::min(geom_.box_.min_lat_, current_tpt_.lat_) ;
        geom_.box_.max_lon_ = std::max(geom_.box_.max_lon_, current_tpt_.lon_) ;
        geom_.box_.max_lon_ = std::max(geom_.box_.min_lon_, current_tpt_.lat_) ;

        ctx_.push(TrackPointContext) ;

    } else if ( qname == "trkseg" ) {
        current_seg_ = TrackSegment() ;
        ctx_.push(TrackSegmentContext) ;
    } else if ( qname == "trk" ) {
        current_track_ = Track() ;
        ctx_.push(TrackSegmentContext) ;
    }

}

void GpxParser::endElement(const std::string &qname) {
    if ( qname == "gpx" ) {
        ctx_.pop() ;
    } else if ( qname == "trkpt" ) {
        ctx_.pop() ;
        current_seg_.pts_.emplace_back(current_tpt_) ;
    } else if ( qname == "wpt" ) {
        ctx_.pop() ;
        geom_.wpts_.emplace_back(current_wpt_) ;
    } else if ( qname == "trkseg" ) {
        ctx_.pop() ;
        current_track_.segments_.emplace_back(current_seg_) ;
    } else if ( qname == "trk" ) {
        ctx_.pop() ;
        geom_.tracks_.emplace_back(current_track_) ;
    } else if ( qname == "name" ) {
        Context ctx = ctx_.top() ;
        if ( ctx == WayPointContext ) current_wpt_.name_ = text_ ;
        else if ( ctx == TrackSegmentContext ) current_seg_.name_ = text_ ;
        else if ( ctx == TrackContext ) current_track_.name_ = text_ ;
    } else if ( qname == "desc" ) {
        Context ctx = ctx_.top() ;
        if ( ctx == WayPointContext ) current_wpt_.desc_ = text_ ;
    } else if ( qname == "ele" ) {
        Context ctx = ctx_.top() ;
        if ( ctx == WayPointContext ) current_wpt_.ele_ = stod(text_) ;
        else if ( ctx == TrackPointContext ) current_tpt_.ele_ = stod(text_) ;
    }

    text_.clear() ;
}

void GpxParser::characters(const string &text) {
    text_ = boost::trim_copy(text) ;
}
